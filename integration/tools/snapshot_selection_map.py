"""Convert the approved map and freeze a background-only child of the 1.20 intake."""
import argparse
from array import array
from io import BytesIO
import json
from pathlib import Path
import struct
import sys

from binary_delta import sha
from build_remaster import data_json, put
from build_native_remaster import runtime_additions
from snapshot_selection_component import checked, relative

PARENT = '44eabd4c4dc62deef753b6542fa6030d348ffa81d88ace79ebd19abec74cb392'
PNG = '57fccea221b9b1befa589f0e7e756b3e73c77dd269463b06bcf0e36e298ba7d4'
OLD = 'e8cf73382fcadbf2334697c33d5aab348d7f3e26f36b7525cd29153ec322cd02'
MAP = 'rs/Remaster/Selection/selection.dwcard'
HEADER = 'DDrawCompat/Remaster/SelectionAssetPins.h'


def convert(data):
    import PIL
    from PIL import Image
    if PIL.__version__ != '13.0.0':
        raise ValueError('Pillow 13.0.0 is required for this developer-only conversion')
    with Image.open(BytesIO(data)) as im:
        if (im.format, im.mode, im.size, im.n_frames) != ('PNG', 'RGB', (2560, 1440), 1):
            raise ValueError('Approved map must be a single-frame 2560x1440 RGB PNG')
        rgb = im.tobytes()
    pixels = array('H', ((r >> 3) << 11 | (g >> 2) << 5 | (b >> 3)
                         for r, g, b in zip(rgb[::3], rgb[1::3], rgb[2::3])))
    if sys.byteorder != 'little':
        pixels.byteswap()
    return b'DWCARD1\0' + struct.pack('<II', 2560, 1440) + pixels.tobytes()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action', choices=('prepare', 'freeze'))
    for name in ('parent', 'png', 'work'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--out', type=Path)
    a = parser.parse_args()
    if a.action == 'prepare' and a.work.exists():
        raise ValueError('Use a new, short Windows build directory')
    if a.action == 'freeze' and (a.out is None or a.out.exists()):
        raise ValueError('Use a new immutable intake directory')
    lock = json.loads(checked(a.parent / 'source-lock.json', PARENT))
    rows = {r['path']: r for r in lock['files']}
    for name, row in rows.items():
        checked(a.parent / relative(name), row['sha256'], row['size'])
    manifest = json.loads((a.parent / 'qfr-source.json').read_bytes())
    pins = manifest['production_files']
    old_row, = [r for r in runtime_additions(lock) if r['target'] == MAP]
    if old_row['sha256'] != OLD:
        raise ValueError('Unexpected existing map')
    card = convert(checked(a.png, PNG))
    digest = sha(card)
    old_header = checked(a.parent / 'wrapper-source' / HEADER, pins[HEADER])
    old_pattern = f'mapHash[]="{OLD}"'.encode()
    if old_header.count(old_pattern) != 1 or digest == OLD:
        raise ValueError('Expected a single old map binding and different map pixels')
    header = old_header.replace(old_pattern, f'mapHash[]="{digest}"'.encode())
    receipt = dict(schema='selection-map-only-1', parent_lock_sha256=PARENT,
                   approved_png_sha256=PNG, previous_map_sha256=OLD,
                   map_sha256=digest, width=2560, height=1440,
                   format='DWCARD1, RGB565LE, no scaling or PNG metadata',
                   previous_pin_header_sha256=pins[HEADER], pin_header_sha256=sha(header),
                   production_source_delta=[HEADER], runtime_payload_delta=[MAP, 'ddraw.dll'])
    if a.action == 'prepare':
        for name, pin in pins.items():
            put(a.work / relative(name), header if name == HEADER else
                checked(a.parent / 'wrapper-source' / name, pin))
        put(a.work / 'selection.dwcard', card)
    else:
        checked(a.work / 'selection.dwcard', digest)
        for name, pin in pins.items():
            checked(a.work / relative(name), sha(header) if name == HEADER else pin)
        wrapper = (a.work / 'Release/ddraw.dll').read_bytes()
        if not wrapper.startswith(b'MZ') or digest.encode() not in wrapper or OLD.encode() in wrapper:
            raise ValueError('Rebuilt wrapper does not contain the exact new map pin')
        mapped_pins = {**pins, HEADER: sha(header)}
        for name, row in rows.items():
            data = {f'wrapper-source/{HEADER}': header, old_row['input']: card,
                    'ddraw.dll': wrapper}.get(name)
            if name == 'qfr-source.json':
                continue
            if data is None:
                data = checked(a.parent / name, row['sha256'], row['size'])
            put(a.out / name, data)
        receipt.update(wrapper_sha256=sha(wrapper))
        put(a.out / 'selection-map-conversion.json', data_json(receipt))
        manifest.update(production_files=mapped_pins, wrapper_sha256=sha(wrapper),
                        map_conversion_sha256=sha(data_json(receipt)),
                        live_test='not performed on map-swap child')
        put(a.out / 'qfr-source.json', data_json(manifest))
        lock.update(previous_intake_sha256=PARENT, wrapper_sha256=sha(wrapper),
                    source_manifest_sha256=sha(data_json(manifest)),
                    live_acceptance='private parent coverage only; map smoke pending',
                    selection_map_conversion_sha256=sha(data_json(receipt)))
        lock['runtime_additions'] = [{**r, 'sha256': digest, 'size': len(card)} if r['target'] == MAP else r
                                     for r in lock['runtime_additions']]
        lock['files'] = [dict(path=f.relative_to(a.out).as_posix(), sha256=sha(f.read_bytes()), size=f.stat().st_size)
                         for f in sorted(a.out.rglob('*')) if f.is_file()]
        runtime_additions(lock)
        put(a.out / 'source-lock.json', data_json(lock))
        receipt.update(lock_sha256=sha(data_json(lock)))
    print(json.dumps(receipt, indent=2))


if __name__ == '__main__':
    main()
