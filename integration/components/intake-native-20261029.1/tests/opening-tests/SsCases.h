namespace {
void runSsCases(){using namespace MenuCanvasTest;
 {HdFixture f;Remaster::managed=Remaster::valid=true;Remaster::readiness=0;
  check(!binkOpen(nullptr,0)&&closeCalls==1&&!movies.count(hdHandle),"managed HD handle cannot escape before full functional startup readiness");
  Remaster::readiness=63;f.open();check(bool(movies[hdHandle]->hd),"managed HD handle admitted after complete renderer readiness");
  Remaster::managed=Remaster::valid=false;Remaster::readiness=0;}
 {HdFixture f;put(hdHandle+8,uint32_t{281});put(hdHandle+0x14,uint32_t{30});scalar(0x749020+4*0x5C7,42);put(0x74A9F0,uint32_t{42});f.open();
  check(movies[hdHandle]->profile==&hdProfiles[2],"S&S owns281/30 profile, not another startup movie");
  Qfr::recording=false;
  check(f.drawHd()==0&&codecCalls==1&&observedFlags==0x8000000A&&lease.written,"S&S decodes once privately and writes native identity without recording");
  f.unlock();check(moviePublication.interactive,"S&S input publication follows matching Unlock without recording");
  f.arm();check(f.drawHd()==0&&codecCalls==2&&observedFlags==10&&f.pixel(2)==0x07E0,"S&S selective history retains previous pixels without recording");
  binkClose(reinterpret_cast<void*>(hdHandle));check(closeCalls==1&&!movies.count(hdHandle),"S&S lifetime closes once");Qfr::recording=true;
 }
 {HdFixture f;put(hdHandle+8,uint32_t{281});put(hdHandle+0x14,uint32_t{30});scalar(0x749020+4*0x5C7,42);f.open();
  check(f.drawHd()==-1&&!codecCalls,"S&S header cannot borrow Paramount resource binding");}
 {HdFixture f;put(hdHandle+8,uint32_t{281});put(hdHandle+0x14,uint32_t{30});scalar(0x749020+4*0x5C7,42);put(0x74A9F0,uint32_t{42});f.open();
  check(copy(fakeCopy,0x40F2ED,reinterpret_cast<void*>(hdHandle),nativePixels,HdFixture::stride,1440,0,0,10)==-1&&!codecCalls,"S&S original inherited80,60 is separately checked from privateXY0");}
}
}
