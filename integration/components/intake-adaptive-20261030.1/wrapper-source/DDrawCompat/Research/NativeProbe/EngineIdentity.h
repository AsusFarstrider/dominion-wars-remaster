#pragma once
namespace NativeProbe::Identity {
struct Engine {const char* sha;unsigned width,height;bool hybrid;};
inline constexpr Engine engines[]={
{"e81aacb185e755c6478c06516bf0d442b7b95edc1f3a3603bfc7ecda1c777023",800,600,false},
{"2109554006153f4aecf2a53ed5d447491b3efcf5cf10dead50ee8ed19775b1b6",1280,960,false},
{"a3bcbfe084d6d6ebccc9af0e384e76d8cd4e1a052aa6decfed96185ba135f4dd",1920,1080,false},
{"de3e3ede7a9b4ce65a891201547f36413f3a880e4df3f28fb8a81e51f687abc6",2560,1440,false},
{"77a99e3bf0a2d8704f4060e5ce584514daa03bb44891fd3336a7d66460efe179",960,600,true},
};
}
