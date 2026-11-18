#pragma once
namespace {
void installationChecks() {
    using namespace SelectionRestoration;
    wchar_t exe[32768]{};check(GetModuleFileNameW(nullptr,exe,32768)!=0,"test module path");
    auto folder=std::filesystem::path(exe).parent_path()/L"rs/Remaster/Selection";
    check(!std::filesystem::exists(folder),"fresh fixture output folder only");std::filesystem::create_directories(folder);
    const std::filesystem::path assets="E:/Codex/research/dominion-rendering/selection-reference-free-assets-20261102-01";
    Remaster::managed=Remaster::valid=true;
    auto reset=[] {catalog.reset();map.reset();flags=0;};
    auto write=[&](const char* name,const std::vector<uint8_t>& bytes){std::ofstream f(folder/name,std::ios::binary|std::ios::trunc);f.write(reinterpret_cast<const char*>(bytes.data()),bytes.size());};
    reset();install();check(!catalog&&!flags,"missing flags retains original at startup");
    write("selection.flags",{0});reset();install();check(!catalog&&!flags,"explicit disabled retains original");
    write("selection.flags",{16});reset();install();check(!catalog&&!flags,"unsupported flags fail safely");
    write("selection.flags",{15});reset();install();check(!catalog&&!flags,"missing catalog fails safely");
    write("selection.dwselect",{'D','W','S','S','E','L','1',0});reset();install();check(!catalog&&!flags,"old or malformed payload fails safely");
    std::filesystem::copy_file(assets/"selection.dwselect",folder/"selection.dwselect",std::filesystem::copy_options::overwrite_existing);
    reset();install();check(!catalog&&!flags,"required enabled map missing retains original");
    std::filesystem::copy_file(assets/"selection.dwcard",folder/"selection.dwcard");reset();install();
    check(catalog&&map&&flags==15,"new pinned files enable the unchanged feature defaults");
    auto corrupted=file(assets/"selection.dwselect",32*1024*1024);corrupted.back()^=1;write("selection.dwselect",corrupted);reset();install();
    check(!catalog&&!flags,"content hash tampering refuses all optional selection overrides");
    reset();Remaster::managed=Remaster::valid=false;
}
}
