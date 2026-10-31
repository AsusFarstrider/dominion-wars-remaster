from pathlib import Path
import hashlib
here=Path(__file__).resolve().parent
p=Path('E:/Codex/research/dominion-rendering/esc-main-menu-source-v1/DDrawCompat/Remaster/OrderedPaint.h')
s=p.read_text();s=s.replace('#include "ButtonImage.h"','// Shared unchanged SpriteFrame definition supplied by the child header.')
s=s.replace('class OrderedPaint {','class ReferenceOrderedPaint {')
(here/'ReferenceOrderedPaint.h').write_text(s)
(here/'reference-provenance.json').write_text('{"parent_sha256":"'+hashlib.sha256(p.read_bytes()).hexdigest()+'","changes":"Class renamed; duplicate local shared-type include removed; original composition body unchanged."}\n')
