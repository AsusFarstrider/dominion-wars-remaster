#pragma once
#include <array>
#include <cstdint>
#include <cstring>
#include <memory>
#include <optional>
#include <vector>
namespace Remaster {
struct SpriteFrame {
    std::shared_ptr<const std::vector<uint8_t>> storage;size_t offset=0;unsigned width=0,height=0;
    bool valid()const noexcept{return storage&&width&&height&&width<=4096&&height<=4096&&offset<=storage->size()&&uint64_t(width)*height*4<=storage->size()-offset;}
    const uint8_t* pixels()const noexcept{return valid()?storage->data()+offset:nullptr;}
};
struct ButtonBank {
    uint32_t idle=0;std::array<SpriteFrame,35> frames;
    static std::optional<ButtonBank> parse(std::vector<uint8_t> bytes,uint32_t expectedIdle) {
        if(bytes.size()<160||memcmp(bytes.data(),"DWBANK1\0",8))return std::nullopt;
        auto word=[&](size_t p){uint32_t x=0;memcpy(&x,bytes.data()+p,4);return x;};
        const auto w=word(8),h=word(12),count=word(16);
        if(w!=720||h!=400||count!=35||bytes.size()!=160+uint64_t(w)*h*4*35)return std::nullopt;
        for(unsigned i=0;i<35;++i)if(word(20+i*4)!=expectedIdle+i)return std::nullopt;
        ButtonBank bank;bank.idle=expectedIdle;auto owned=std::make_shared<const std::vector<uint8_t>>(std::move(bytes));
        for(unsigned i=0;i<35;++i)bank.frames[i]={owned,160+size_t(i)*w*h*4,w,h};return bank;
    }
};
}
