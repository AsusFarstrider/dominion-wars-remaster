#pragma once
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <map>
#include <string>
#include <vector>

namespace Qfr {
struct Delta {
    uint64_t stream=0,snapshot=0,base=0;size_t packed=0;
    std::string encoding;std::vector<unsigned char> data;
};
inline void word(std::vector<unsigned char>& out,uint32_t n){for(unsigned i=0;i<4;++i)out.push_back(static_cast<unsigned char>(n>>(i*8)));}
// Called under the recorder's pixel lock, held until submission preserves dependency order.
class FrameCache {
    struct Frame {uint64_t stream,snapshot,used;std::vector<unsigned char> bytes;};
    std::map<std::string,Frame> frames;size_t bytes=0,limit;uint64_t stream=0,snapshot=0,clock=0;
public:
    explicit FrameCache(size_t budget):limit(budget){}
    Delta encode(const std::string& key,std::vector<unsigned char> current){
        Delta out;out.snapshot=++snapshot;out.packed=current.size();
        auto it=frames.find(key);
        if(it!=frames.end()&&it->second.bytes.size()==current.size()){
            const auto& previous=it->second.bytes;out.stream=it->second.stream;out.base=it->second.snapshot;
            size_t at=0;
            while(at<current.size()){
                const auto n=std::min(size_t(64),current.size()-at);
                if(memcmp(current.data()+at,previous.data()+at,n)==0){at+=n;continue;}
                const auto start=at;at+=n;
                while(at<current.size()){
                    const auto next=std::min(size_t(64),current.size()-at);
                    if(memcmp(current.data()+at,previous.data()+at,next)==0)break;at+=next;
                }
                word(out.data,uint32_t(start));word(out.data,uint32_t(at-start));
                out.data.insert(out.data.end(),current.begin()+start,current.begin()+at);
            }
            out.encoding=out.data.empty()?"unchanged":"byte_runs_v1";
            if(out.data.size()>=current.size()){out.encoding="full";out.base=0;out.data=current;}
            it->second.bytes=std::move(current);it->second.snapshot=out.snapshot;it->second.used=++clock;
        }else{
            if(it!=frames.end()){bytes-=it->second.bytes.size();frames.erase(it);}
            out.stream=++stream;out.encoding="full";out.data=current;
            bytes+=current.size();frames.emplace(key,Frame{out.stream,out.snapshot,++clock,std::move(current)});
        }
        while(bytes>limit&&!frames.empty()){
            auto oldest=std::min_element(frames.begin(),frames.end(),[](const auto& a,const auto& b){return a.second.used<b.second.used;});
            bytes-=oldest->second.bytes.size();frames.erase(oldest);
        }
        return out;
    }
};
}
