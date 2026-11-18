#pragma once
#include <cstdint>
#include <map>
#include <regex>
#include <stdexcept>
#include <string>
namespace Remaster {
// Bounded flat JSON for the two immutable package records. No nesting,
// duplicate keys, implicit type conversion, trailing content or silent fields.
struct Atom {enum Kind{string,integer,boolean,nullValue} kind=nullValue;std::string text;};
using Object=std::map<std::string,Atom>;
class FlatJson {
    const std::string& s;size_t p=0;
    [[noreturn]] void bad(){throw std::runtime_error("invalid flat JSON package record");}
    void ws(){while(p<s.size()&&(s[p]==' '||s[p]=='\r'||s[p]=='\n'||s[p]=='\t'))++p;}
    bool take(char c){ws();if(p<s.size()&&s[p]==c){++p;return true;}return false;}
    std::string string(){if(!take('"'))bad();std::string out;
        while(p<s.size()){unsigned char c=s[p++];if(c=='"')return out;if(c<32)bad();
            if(c=='\\'){if(p==s.size())bad();c=s[p++];
                if(c=='u'){unsigned v=0;for(unsigned i=0;i<4;++i){if(p==s.size())bad();const char h=s[p++];v*=16;
                    if(h>='0'&&h<='9')v+=h-'0';else if(h>='a'&&h<='f')v+=h-'a'+10;else if(h>='A'&&h<='F')v+=h-'A'+10;else bad();}
                    if(v<32||v>126)bad();c=static_cast<unsigned char>(v);
                }else if(c!='"'&&c!='\\'&&c!='/')bad();}
            if(c>126||out.size()>=512)bad();out+=char(c);
        }bad();}
    Atom value(){ws();if(p<s.size()&&s[p]=='"')return {Atom::string,string()};
        for(const auto& token:{std::string("true"),std::string("false"),std::string("null"),std::string("1")})
            if(s.compare(p,token.size(),token)==0){p+=token.size();return {token=="null"?Atom::nullValue:token=="1"?Atom::integer:Atom::boolean,token};}
        bad();}
public:
    explicit FlatJson(const std::string& text):s(text){}
    Object parse(){if(s.empty()||s.size()>16384)bad();Object out;if(!take('{'))bad();
        if(!take('}'))for(;;){const auto key=string();if(key.empty()||key.size()>64||!take(':'))bad();
            const auto atom=value();if(out.size()>=24||!out.emplace(key,atom).second)bad();
            if(take('}'))break;if(!take(','))bad();}
        ws();if(p!=s.size())bad();return out;}
};
inline const Atom& field(const Object& o,const char* k,Atom::Kind type){const auto i=o.find(k);
    if(i==o.end()||i->second.kind!=type)throw std::runtime_error(std::string("missing/type package field: ")+k);return i->second;}
inline std::string text(const Object& o,const char* k){return field(o,k,Atom::string).text;}
inline bool flag(const Object& o,const char* k){return field(o,k,Atom::boolean).text=="true";}
inline bool matches(const std::string& s,const char* re){return std::regex_match(s,std::regex(re));}
inline bool approvedMovie(const std::string& actual,bool permit,const char* original,const char* replacement){
    if(actual==original)return false;if(permit&&actual==replacement)return true;throw std::runtime_error("movie identity not admitted by package profile");}
struct Identity {std::string label,build,revision,flavor,stage,version;};
inline Identity identity(const std::string& bytes){const auto o=FlatJson(bytes).parse();
    if(o.size()!=10||field(o,"schema",Atom::integer).text!="1"||text(o,"product")!="Dominion Wars Remastered"||text(o,"original_game_version")!="1.04")throw std::runtime_error("identity schema/product/base");
    Identity v;v.label=text(o,"in_game_label");v.build=text(o,"integration_build_id");v.revision=text(o,"source_revision");v.flavor=text(o,"flavor");v.stage=text(o,"release_stage");
    if(!matches(v.build,"int-[0-9]{8}\\.[1-9][0-9]*(-[a-z0-9-]+)?")||!matches(v.revision,"[a-f0-9]{40}")||v.flavor.find_first_not_of(' ')==std::string::npos||text(o,"version_decision_id").find_first_not_of(' ')==std::string::npos||
       (v.stage!="experimental"&&v.stage!="alpha"&&v.stage!="beta"&&v.stage!="release-candidate"&&v.stage!="stable"))throw std::runtime_error("identity metadata");
    const auto version=o.find("remaster_version");if(version==o.end())throw std::runtime_error("identity version missing");
    if(version->second.kind==Atom::nullValue){if(v.label!="Version 1.04")throw std::runtime_error("original label mismatch");}
    else {v.version=text(o,"remaster_version");if(!matches(v.version,"(1\\.(0[5-9]|[1-9][0-9])|([2-9]|[1-9][0-9]+)\\.[0-9]{2})")||v.label!="Remastered "+v.version)throw std::runtime_error("remaster label mismatch");}
    if(v.label.empty()||v.label.size()>80)throw std::runtime_error("label length");return v;
}
struct Options {std::string engine,versionHash;bool recording=false,paramount=false,gizmo=false,ss=false,publisher=false,title=false,main=false,buttons=false,dynamicOutput=false;};
inline Options options(const std::string& bytes){const auto o=FlatJson(bytes).parse();
    const auto profile=text(o,"profile");
    if(o.size()!=12||field(o,"schema",Atom::integer).text!="1"||(profile!="native-1440-v1"&&profile!="native-display-v1"))throw std::runtime_error("rendering schema/profile");
    Options v;v.engine=text(o,"engine_sha256");v.versionHash=text(o,"version_sha256");const auto mode=text(o,"recording");
    v.dynamicOutput=profile=="native-display-v1";
    const auto expected=v.dynamicOutput?"e81aacb185e755c6478c06516bf0d442b7b95edc1f3a3603bfc7ecda1c777023":"de3e3ede7a9b4ce65a891201547f36413f3a880e4df3f28fb8a81e51f687abc6";
    if(v.engine!=expected||!matches(v.versionHash,"[a-f0-9]{64}")||(mode!="off"&&mode!="compact"))throw std::runtime_error("rendering identity/recording");
    v.recording=mode=="compact";v.paramount=flag(o,"paramount_hd");v.gizmo=flag(o,"gizmo_hd");v.ss=flag(o,"ss_hd");v.publisher=flag(o,"publisher_card");v.title=flag(o,"title_card");v.main=flag(o,"main_background");v.buttons=flag(o,"main_buttons");return v;
}
inline Identity boundIdentity(const Options& config,const std::string& bytes,const std::string& actualHash){
    if(actualHash!=config.versionHash)throw std::runtime_error("version.json hash mismatch");return identity(bytes);
}
}
