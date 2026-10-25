"""Native ARM menu skin and font with Chromium touch/gamepad emulation, not physical iPhone."""
import asyncio,functools,json,pathlib,threading
from http.server import SimpleHTTPRequestHandler,ThreadingHTTPServer
from playwright.async_api import async_playwright
ROOT=pathlib.Path(__file__).resolve().parents[1]
class Quiet(SimpleHTTPRequestHandler):
 def log_message(self,*args):pass
async def run(url):
 async with async_playwright() as p:
  browser=await p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox','--disable-dev-shm-usage'])
  page=await browser.new_page(viewport={'width':932,'height':430},is_mobile=True,has_touch=True)
  errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
  await page.add_init_script("window.padTest=null;Object.defineProperty(navigator,'getGamepads',{value:()=>[window.padTest]})")
  await page.goto(url);assert await page.locator('#ui-scale').count()==0;await page.evaluate("localStorage.setItem('destinia.uiScale','75')");await page.locator('#start').tap();await page.wait_for_function('!document.getElementById("pause").disabled',timeout=45000)
  worker=page.workers[0]
  await worker.evaluate("""()=>{paused=true;clearTimeout(timer);const r=engine,g=r.exports.get('Game'),u=r.exports.get('GameUI'),s=(at,...v)=>{const b=new Uint8Array(v.length*2),d=new DataView(b.buffer);v.forEach((n,i)=>d.setUint16(i*2,n,true));r.write(at,b);};r.call(r.exports.get('GameCanvas_init'),[r.exports.get('GameCanvas')]);r.call(r.exports.get('Map_loadData'),[2,49,1]);for(let i=0;i<6;i++)r.put32(g+0x4584+4*i,0);s(g+0x48ba,20,15,10,10);s(g+0x48fa,20);s(g+0x7574,65535);r.call(r.exports.get('GameUI_calcHeroStat'));r.call(r.exports.get('openMainmenu'),[0]);s(u+0x76,16);r.put32(r.get32(0x10044240),r.exports.get('GameCanvas'));r.put32(r.gMenu+156,2);r.codex.select(0);r.codex.observe(100,61,true,{maxHP:170,level:2,elite:false,location:{file:'m2_49.mpd',name:'Dungeon Prison - 4'}});r.codex.observe(101,61,true);paused=false;tick();}""")
  async def tap(x,y,scale=100):
   x,y=await worker.evaluate('([x,y,p])=>{const s=Math.min(p/100,1);return [Math.floor(x*s+200*(1-s)),Math.floor(y*s+120*(1-s))]}',[x,y,scale])
   b=await page.locator('#game').bounding_box();await page.touchscreen.tap(b['x']+(x+.5)*b['width']/400,b['y']+(y+.5)*b['height']/240);await page.wait_for_timeout(100)
  for scale in [100]:
   assert await worker.evaluate('engine.uiScale')==100
   await tap(320,12,scale);assert await worker.evaluate('engine.codexOpen')
   assert await worker.evaluate('engine.codex.catalog()[60].kills')==2
   snapshot=await worker.evaluate("Array.from(engine.read(engine.exports.get('Game'),32056)).join(',')")
   # Opening selected first discovered soldier (index60/page1).
   await page.screenshot(path=f'/tmp/destinia-native-codex-{scale}.png')
   # Instant consecutive canvas taps are handled without ARM-frame coalescing.
   await tap(270,199,scale);assert await worker.evaluate('engine.codexDetailsOpen')
   assert await worker.evaluate('engine.codexDetailData().sections[0].id')=='drops'
   await tap(275,225,scale);assert await worker.evaluate('engine.codexDetailData().sections[engine.codexDetailPage].id')=='spawns'
   await page.screenshot(path=f'/tmp/destinia-native-details-{scale}.png')
   assert snapshot==await worker.evaluate("Array.from(engine.read(engine.exports.get('Game'),32056)).join(',')")
   await tap(290,35,scale);assert await worker.evaluate('!engine.codexDetailsOpen&&engine.codexOpen')
   await tap(290,35,scale);assert await worker.evaluate('!engine.codexOpen')
   # Click all original tabs and confirm real native state, including craft/pets.
   for tab in range(9):
    await tap(113+23*tab,12,scale)
    await page.wait_for_timeout(160)
    actual=await worker.evaluate("engine.get32(engine.exports.get('GameUI')+0x7c)>>>28");assert actual==tab,(scale,tab,actual)
   await tap(320,12,scale)
   assert await worker.evaluate("engine.codex.catalog()[60].kills")==2
   await tap(290,35,scale)
  # The entry remains clickable while browsing the ordinary inventory body.
  await tap(159,12,100)
  await worker.evaluate("engine.put32(engine.exports.get('GameUI')+0x7c,0x20000001)")
  await tap(320,12,100);assert await worker.evaluate('engine.codexOpen')
  await tap(290,35,100);assert await worker.evaluate("engine.get32(engine.exports.get('GameUI')+0x7c)===0x20000001")
  await tap(297,12,100)
  await worker.evaluate('engine.key(7,true);engine.key(7,false)')
  assert await worker.evaluate('engine.codexTabSelected&&!engine.codexOpen')
  await page.screenshot(path='/tmp/destinia093-codex-preview.png')
  await tap(270,199,100);assert await worker.evaluate('engine.codexOpen&&engine.codexDetailsOpen')
  await tap(290,35,100);await tap(290,35,100)
  # Actual site gamepad plumbing with a synthetic standard gamepad.
  await page.evaluate("window.padTest={index:0,id:'QA EMULADO',connected:true,mapping:'standard',axes:[0,0,0,0],buttons:Array.from({length:17},()=>({pressed:false,value:0}))}")
  await tap(320,12,100)
  async def button(index):
   await page.evaluate('(i)=>padTest.buttons[i].pressed=true',index);await page.wait_for_timeout(150);await page.evaluate('(i)=>padTest.buttons[i].pressed=false',index);await page.wait_for_timeout(150)
  await button(0);assert await worker.evaluate('engine.codexDetailsOpen')
  await button(1);assert await worker.evaluate('!engine.codexDetailsOpen&&engine.codexOpen')
  await page.evaluate('window.dispatchEvent(new Event("blur"))');await page.evaluate('window.dispatchEvent(new Event("focus"))');await page.wait_for_timeout(120)
  assert await page.locator('#error').is_hidden(),await page.locator('#error-text').inner_text()
  assert not errors,errors
  print(json.dumps({'result':'PASS','native_skin_font':True,'scales':[100],'original_tabs':9,'kills':2,'gamepad':'emulated','errors':errors}))
  await browser.close()
if __name__=='__main__':
 server=ThreadingHTTPServer(('127.0.0.1',0),functools.partial(Quiet,directory=str(ROOT/'dist')));threading.Thread(target=server.serve_forever,daemon=True).start()
 try:asyncio.run(run(f'http://127.0.0.1:{server.server_port}'))
 finally:server.shutdown();server.server_close()
