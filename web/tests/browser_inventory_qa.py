"""Real ARM/filter UI with emulated gamepad; not physical Windows/iPhone QA."""
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
  await page.goto(url);assert await page.locator('#ui-scale').count()==0;await page.locator('#start').tap();await page.wait_for_function('!document.getElementById("pause").disabled',timeout=45000)
  worker=page.workers[0]
  await worker.evaluate("""()=>{paused=true;clearTimeout(timer);const r=engine,g=r.exports.get('Game'),u=r.exports.get('GameUI'),s=(at,...v)=>{const b=new Uint8Array(v.length*2),d=new DataView(b.buffer);v.forEach((n,i)=>d.setUint16(i*2,n,true));r.write(at,b);};self.invShort=s;r.call(r.exports.get('GameCanvas_init'),[r.exports.get('GameCanvas')]);r.call(r.exports.get('Map_loadData'),[2,49,1]);for(let i=0;i<6;i++)r.put32(g+0x4584+4*i,0);s(g+0x48ba,20,15,10,10);s(g+0x48fa,20);s(g+0x7574,65535);r.call(r.exports.get('GameUI_calcHeroStat'));r.call(r.exports.get('setMyGold'),[20000]);self.invA=r.call(r.exports.get('GameUI_getNewEquipByIdx'),[2]);self.invB=r.call(r.exports.get('GameUI_getNewEquipByIdx'),[2]);r.write(g+0x2ed6+20*invB,r.read(g+0x2ed6+20*invA,20));for(let bag=0;bag<4;bag++)s(g+0x4338+bag*0x92,4,0,bag?0:2);s(g+0x433e,0,invA,1,0,invB,1);r.call(r.exports.get('openMainmenu'),[0]);s(u+0x76,16);for(const k of [7,9,6])r.call(r.exports.get('MainMenu_mKeyPressed'),[k]);r.put32(r.get32(0x10044240),r.exports.get('GameCanvas'));r.put32(r.gMenu+156,2);paused=false;tick();}""")
  await page.keyboard.press('F7');await page.wait_for_function('true')
  assert await worker.evaluate('engine.inventory.open')
  before=await worker.evaluate('Array.from(engine.read(engine.exports.get("Game")+0x4338,4*0x92))')
  async def touch(x,y,scale=100):
   point=await worker.evaluate('([x,y,scale])=>{const p={x:20,y:13,w:360,h:214},s=Math.min(scale/100,400/360,240/214);return {x:Math.floor(x*s+400*(1-s)/2),y:Math.floor(y*s+240*(1-s)/2)}}',[x,y,scale])
   rect=await page.locator('#game').bounding_box();await page.touchscreen.tap(rect['x']+(point['x']+.5)*rect['width']/400,rect['y']+(point['y']+.5)*rect['height']/240)
  # All filters/direction and clear are real canvas taps, not DOM controls.
  await touch(30,45);assert await worker.evaluate('engine.inventory.panel.query.type===0')
  await touch(30,45);assert await worker.evaluate('engine.inventory.panel.view.length===0')
  await touch(285,68);assert await worker.evaluate('engine.inventory.panel.query.type===-1')
  await touch(210,45);assert await worker.evaluate('engine.inventory.panel.levelChoice===1')
  await touch(300,45);assert await worker.evaluate('engine.inventory.panel.view.length===0')
  await touch(285,68);await touch(50,68);await touch(200,68)
  assert await worker.evaluate('engine.inventory.panel.query.sort===1&&engine.inventory.panel.query.descending')
  assert await worker.evaluate('Array.from(engine.read(engine.exports.get("Game")+0x4338,4*0x92))')==before
  await page.screenshot(path='/tmp/destinia091-browser-inventory100.png')
  await touch(285,68)
  await page.evaluate("window.padTest={index:0,id:'QA EMULADO',connected:true,mapping:'standard',axes:[0,0,0,0],buttons:Array.from({length:17},()=>({pressed:false,value:0}))}")
  await page.wait_for_timeout(120)
  async def button(index):
   await page.evaluate('(i)=>padTest.buttons[i].pressed=true',index);await page.wait_for_timeout(160);await page.evaluate('(i)=>padTest.buttons[i].pressed=false',index);await page.wait_for_timeout(160)
  # Select second identical item by touch, then return with the physical action.
  await touch(100,120)
  assert await worker.evaluate('engine.inventory.panel.selectedPool===invB')
  await button(0)
  assert await worker.evaluate('!engine.inventory.open&&engine.get32(engine.exports.get("Game")+0x4584)===engine.exports.get("Game")+0x2ed6+20*invB')
  assert await worker.evaluate('engine.get32(engine.exports.get("GameUI")+0x2148)===0')
  await button(0)
  assert await worker.evaluate('engine.get32(engine.exports.get("GameUI")+0x2148)===engine.exports.get("Game")+0x2ed6+20*invB')
  await page.evaluate('window.dispatchEvent(new Event("blur"))');await page.evaluate('window.dispatchEvent(new Event("focus"))');await page.wait_for_timeout(100)
  await button(1);assert await worker.evaluate('!engine.inventory.open')
  assert await page.locator('#error').is_hidden(),await page.locator('#error-text').inner_text()
  assert not errors,errors
  print(json.dumps({'result':'PASS','scope':'ARM real, touch100, gamepad emulado, foco, identidade duplicada, confirmacao nativa','errors':errors}))
  await browser.close()
if __name__=='__main__':
 server=ThreadingHTTPServer(('127.0.0.1',0),functools.partial(Quiet,directory=str(ROOT/'dist')));threading.Thread(target=server.serve_forever,daemon=True).start()
 try:asyncio.run(run(f'http://127.0.0.1:{server.server_port}'))
 finally:server.shutdown();server.server_close()
