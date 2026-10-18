"""Classify source-history path references without changing committed bytes."""
import io
import json
from pathlib import Path
import re
import subprocess
import tarfile

ROOT=Path(__file__).resolve().parents[1]
REV='921826392c5b6f5acdc523f4db2429cbad234782'
data=subprocess.check_output(['C:/Program Files/Git/cmd/git.exe','-C',str(ROOT),'archive','--format=tar',REV])
rows=[];personal=[]
with tarfile.open(fileobj=io.BytesIO(data)) as tree:
    for member in tree.getmembers():
        if not member.isfile():continue
        b=tree.extractfile(member).read()
        if b'\0' in b:continue
        text=b.decode('utf-8',errors='replace')
        text=text.replace('\\\\','/').replace('\\','/')
        if re.search(r'(?i)(?:[a-z]:/Users/|/home/|/Users/|[\w.+-]+@(?:gmail|outlook|hotmail)\.)',text):
            personal.append(member.name)
        if re.search(r'(?i)[A-Z]:/(?:Codex|Program Files)/',text):
            categories=[]
            if re.search(r'(?i)[A-Z]:/Codex/',text):categories.append('Historical local fixture/build/provenance location; no personal username')
            if re.search(r'(?i)[A-Z]:/Program Files/',text):categories.append('Standard software-install path')
            rows.append(dict(path=member.name,categories=categories,action='retain frozen source bytes; document environment specificity'))
out=dict(source_revision=REV,reviewed_environment_path_files=len(rows),personal_path_or_contact_files=personal,
         classification=rows,history_rewrite_needed=bool(personal),
         scope='All text blobs in the single commit reachable from approved master; private Codex refs are not public push inputs')
print(json.dumps(out,indent=2))
report=ROOT/'handoffs/release/int-20261018.1/path-review.json'
if report.exists():raise ValueError('Preserve previous report')
with report.open('x',encoding='utf-8') as stream:json.dump(out,stream,indent=2)
