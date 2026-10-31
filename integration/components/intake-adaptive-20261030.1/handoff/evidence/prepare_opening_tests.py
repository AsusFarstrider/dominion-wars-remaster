import shutil
from pathlib import Path
here=Path(__file__).resolve().parent
source=Path('E:/Codex/research/dominion-rendering/esc-menu-perf-source-v1/production-validation-sources/opening-tests')
target=here/'opening-tests';shutil.copytree(source,target)
shutil.copyfile(here/'AdaptiveCases.h',target/'AdaptiveCases.h')
p=target/'OpeningControllerTests.cpp';s=p.read_text()
anchor='#include "HdCases.h"';assert anchor in s
s=s.replace(anchor,anchor+'\n#include "AdaptiveCases.h"')
s=s.replace('    runHdCases();','    runHdCases();\n    runAdaptiveCases();')
p.write_text(s)
