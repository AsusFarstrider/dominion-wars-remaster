from pathlib import Path
p=Path('E:/Codex/research/dominion-rendering/hd-video-work-01/DDrawCompat/Research/OpeningFlow/OpeningFlow.cpp')
s=p.read_text()
s=s.replace('bool interactive=false;};','bool interactive=false;uint32_t hdHandle=0;uint64_t hdLifetime=0;};',1)
s=s.replace('struct Movie {bool hdBound=false;','struct Movie {unsigned hdReceipts=0;bool hdBound=false;')
s=s.replace('if(!moviePublication.interactive)error("video_unlock_or_owner_failed");','''if(moviePublication.hdHandle) {
                    const auto i=movies.find(moviePublication.hdHandle);
                    moviePublication.interactive=moviePublication.interactive&&i!=movies.end()&&i->second->lifetime==moviePublication.hdLifetime&&
                        i->second->hd&&i->second->hd->initialized&&!i->second->hd->quarantined&&
                        t.w==i->second->hd->width&&t.h==i->second->hd->height;
                    trace("native_hd_unlock",n("lifetime",moviePublication.hdLifetime)+n("token",lease.token)+n("surface",object)+n("hr",hr)+
                        ",\\"published\\":"+(moviePublication.interactive?"true":"false"));
                }
                if(!moviePublication.interactive)error("video_unlock_or_owner_failed");''')
# Put HD validation before generic Unlock logging too.
start=s.index('                trace("native_opening_video_unlock"');end=s.index('                if(moviePublication.hdHandle)',start)
old=s[start:end];s=s[:start]+s[end:];pos=s.index('                if(!moviePublication.interactive)error("video_unlock_or_owner_failed");',start);s=s[:pos]+old+s[pos:]
s=s.replace('written=history.identity(view(lease.d));\n            if(written){surfaces[t.surface].scaled=true;moviePublication={++serial,epoch,c,t.surface,false};lease.written=true;}',
'''written=history.identity(view(lease.d));
            if(written){surfaces[t.surface].scaled=true;moviePublication={++serial,epoch,c,t.surface,false,pointer,owned->lifetime};lease.written=true;
                const auto frame=value(pointer+12);
                if(!owned->hdReceipts||frame==112||(frame>=220&&owned->hdReceipts<3)) {
                    ++owned->hdReceipts;
                    const size_t bytes=size_t(history.pitch)*history.height;
                    std::vector<unsigned char> packed(bytes);
                    for(unsigned row=0;row<history.height;++row)std::memcpy(packed.data()+size_t(row)*history.pitch,
                        static_cast<unsigned char*>(lease.d.lpSurface)+size_t(row)*lease.d.lPitch,history.pitch);
                    unsigned char sourceDigest[32]{},targetDigest[32]{};
                    const bool hashed=BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,static_cast<PUCHAR>(history.data()),ULONG(bytes),sourceDigest,32)>=0&&
                        BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,packed.data(),ULONG(bytes),targetDigest,32)>=0;
                    const bool equal=hashed&&!memcmp(sourceDigest,targetDigest,32);
                    const char* hex="0123456789abcdef";std::string a,b;
                    for(unsigned index=0;index<32;++index){a+=hex[sourceDigest[index]>>4];a+=hex[sourceDigest[index]&15];b+=hex[targetDigest[index]>>4];b+=hex[targetDigest[index]&15];}
                    trace("native_hd_pixels",n("frame",frame)+n("lifetime",owned->lifetime)+n("token",token)+n("bytes",bytes)+
                        ",\\"source_sha256\\":"+Qfr::quote(a)+",\\"target_sha256\\":"+Qfr::quote(b)+",\\"equal\\":"+(equal?"true":"false"));
                    if(!equal){history.quarantined=true;lease.written=false;moviePublication.interactive=false;error("HD_pixel_identity_failed");}
                }
            }''')
s=s.replace('moviePublication.interactive=false;error("HD_provider_exception_no_legacy_fallback");','moviePublication.interactive=false;lease.written=false;error("HD_provider_exception_no_legacy_fallback");')
p.write_text(s)
print('Bounded pixel receipts and matching lifetime publication installed')
