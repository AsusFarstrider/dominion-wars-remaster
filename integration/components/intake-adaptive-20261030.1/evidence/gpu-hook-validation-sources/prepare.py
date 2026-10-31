"""Create a new work child from the immutable failed v1; no game files touched."""
import hashlib,json,shutil
from pathlib import Path
base=Path('E:/Codex/research/dominion-rendering')
parent=base/'production-runtime-source-v1';work=base/'production-runtime-work-02'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert sha(parent/'qfr-source.json')=='4812ccb9c32034f1d5129b7525a0e5c57010927761ca01f23124657233b9e23e'
assert not work.exists()
m=json.loads((parent/'qfr-source.json').read_text())
for r,h in m['opening_flow']['source_files'].items():
    assert sha(parent/r)==h,r
    p=work/r;p.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(parent/r,p)
shutil.copyfile(parent/'qfr-source.json',work/'parent-production-runtime-v1.json')
p=work/'DDrawCompat/Common/VtableHookVisitor.h'
s=p.read_text()
old='if constexpr (!(getCompatVtable<Vtable>().*memberPtr))'
assert s.count(old)==1
s=s.replace(old,'if constexpr (!(getCompatVtable<Vtable>().*memberPtr) && !functionalRoute<memberPtr>())')
anchor='private:\n\ttemplate <auto memberPtr, typename Result, typename FirstParam, typename... Params>'
assert s.count(anchor)==1
s=s.replace(anchor,'''private:
	// These routes contain functional HUD adapters even when no compatibility
	// implementation exists. Their installation must not depend on diagnostics.
	// Keep all other no-Compat slots subject to the original logging/recorder gate.
	template <auto memberPtr>
	static constexpr bool functionalRoute()
	{
		if constexpr (std::is_same_v<decltype(memberPtr), decltype(&IDirect3DDevice3Vtbl::DrawPrimitive)>)
			return memberPtr == &IDirect3DDevice3Vtbl::DrawPrimitive;
		else if constexpr (std::is_same_v<decltype(memberPtr), decltype(&IDirect3DDevice3Vtbl::DrawIndexedPrimitive)>)
			return memberPtr == &IDirect3DDevice3Vtbl::DrawIndexedPrimitive;
		else if constexpr (std::is_same_v<decltype(memberPtr), decltype(&IDirect3DViewport3Vtbl::Clear2)>)
			return memberPtr == &IDirect3DViewport3Vtbl::Clear2;
		else
			return false;
	}

	template <auto memberPtr, typename Result, typename FirstParam, typename... Params>''')
p.write_text(s)
(work/'prepare-receipt.json').write_text(json.dumps(dict(parent=str(parent),parent_manifest_sha256=sha(parent/'qfr-source.json'),source_files_verified=len(m['opening_flow']['source_files']),changed=['DDrawCompat/Common/VtableHookVisitor.h'],game_modified=False,current_modified=False),indent=2)+'\n')
print(work)
