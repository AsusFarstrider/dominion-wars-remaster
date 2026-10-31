"""Mechanically relocate game addresses ONLY in the console fixture copy.
Windows reserves the real0x400000 range in this test host; never unmap it.
Production source is not changed by this preparation.
"""
import hashlib,re,json
from pathlib import Path
here=Path(__file__).resolve().parent
source=Path('E:/Codex/research/dominion-rendering/dynamic-output-work-01/DDrawCompat/Remaster')
s=(source/'OutputMode.cpp').read_text()
assert s.count('0x4B085A')==1
s=s.replace('0x4B085A','0x200B085A').replace('0x400000','0x20000000')
s=s.replace('#include "Runtime.h"','#include <Remaster/Runtime.h>').replace('#include "OutputMode.h"','#include <Remaster/OutputMode.h>').replace('#include "OutputSites.h"','#include "FixtureSites.h"')
(here/'OutputModeFixture.cpp').write_text(s)
s=(source/'OutputSites.h').read_text()
s=re.sub(r'(?m)^ \{(0x[0-9a-f]+),',lambda m:' {'+hex(int(m[1],16)+0x1FC00000)+',',s)
(here/'FixtureSites.h').write_text(s)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
(here/'fixture-provenance.json').write_text(json.dumps(dict(production_cpp_sha256=sha(source/'OutputMode.cpp'),production_sites_sha256=sha(source/'OutputSites.h'),fixture_cpp_sha256=sha(here/'OutputModeFixture.cpp'),fixture_sites_sha256=sha(here/'FixtureSites.h'),changes='Literal engine base/entry/site addresses relocated by0x1FC00000 and local include paths adjusted; installer/binder/bridge logic unchanged; no production edits.'),indent=2)+'\n')
