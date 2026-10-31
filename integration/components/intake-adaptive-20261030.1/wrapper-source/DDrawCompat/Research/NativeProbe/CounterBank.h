#pragma once
// Independent bounded metadata categories: incidental texture churn cannot
// consume the scene/picking/API/presentation category's reserved key capacity.
#include <cstdint>
#include <map>
#include <string>

namespace NativeProbe {
struct Count {uint64_t seen=0,kept=0;int64_t first=0,last=0;};
class CounterBank {
public:
    struct Group {std::map<std::string,Count> keys;uint64_t overflow=0;};
    std::map<std::string,Group> groups;
    uint64_t unknownGroupOverflow=0;
    size_t perGroupLimit=32768,groupLimit=16;
    bool sample(const std::string& key,int64_t time){
        const auto category=key.substr(0,key.find('|'));
        if(!groups.count(category)&&groups.size()>=groupLimit){++unknownGroupOverflow;return false;}
        auto& g=groups[category];
        if(g.keys.size()>=perGroupLimit&&!g.keys.count(key)){++g.overflow;return false;}
        auto& c=g.keys[key];if(!c.seen)c.first=time;c.last=time;++c.seen;
        if(c.kept>=4)return false;++c.kept;return true;
    }
};
}
