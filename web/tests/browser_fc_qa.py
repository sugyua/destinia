"""Controlled real ARM menu/gear scene; browser touch, not a campaign."""
import asyncio,functools,json,pathlib,threading
from http.server import SimpleHTTPRequestHandler,ThreadingHTTPServer
from playwright.async_api import async_playwright
ROOT=pathlib.Path(__file__).resolve().parents[1]
class Quiet(SimpleHTTPRequestHandler):
 def log_message(self,*args):pass
async def run(url):
 async with async_playwright() as p:
  browser=await p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox','--disable-dev-shm-usage'])
  page=await browser.new_page(viewport={'width':430,'height':932},device_scale_factor=2,is_mobile=True,has_touch=True)
  errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
  await page.goto(url);await page.locator('#ui-scale').select_option('75');await page.locator('#start').tap()
  await page.wait_for_function("Number(document.getElementById('game').dataset.frame)>3",timeout=45000)
  worker=page.workers[0]
  await worker.evaluate("""()=>{const r=engine,g=r.exports.get('Game'),s=(at,...v)=>{const b=new Uint8Array(v.length*2),d=new DataView(b.buffer);v.forEach((n,i)=>d.setUint16(i*2,n,true));r.write(at,b);};
   r.call(r.exports.get('GameCanvas_init'),[r.exports.get('GameCanvas')]);r.call(r.exports.get('Map_loadData'),[2,49,1]);r.call(r.exports.get('Mon_createMonster'));
   for(let i=0;i<6;i++)r.put32(g+0x4584+4*i,0);s(g+0x48ba,20,15,10,10);s(g+0x48fa,20);s(g+0x7574,65535);
   const a=r.call(r.exports.get('GameUI_getNewEquipByIdx'),[1]),b=r.call(r.exports.get('GameUI_getNewEquipByIdx'),[2]);r.put32(g+0x4584,g+0x2ed6+20*a);r.call(r.exports.get('GameUI_calcHeroStat'));s(g+0x48f4,400);s(g+0x48f2,50);s(g+0x4338,4,0,2,0,a,1,0,b,1);r.call(r.exports.get('setMyGold'),[20000]);
   self.fcCandidate=g+0x2ed6+20*b;r.call(r.exports.get('openMainmenu'),[0]);s(r.exports.get('GameUI')+0x76,16);r.put32(r.get32(0x10000000+0x44240),r.exports.get('GameCanvas'));
  }""")
  await page.wait_for_timeout(1200)
  assert await page.locator('#error').is_hidden(),await page.locator('#error-text').text_content()
  assert await worker.evaluate('engine.combatPower.status!==null')
  assert await worker.evaluate('engine.combatPower.calculate().current===488')
  await page.screenshot(path='/tmp/destinia09-browser-fc-status.png')
  for key in [7,7,9,6,9,7]:
   await worker.evaluate('(key)=>engine.call(engine.exports.get("MainMenu_mKeyPressed"),[key])',key)
   await page.wait_for_timeout(220)
  await page.wait_for_timeout(500)
  assert await page.locator('#error').is_hidden(),await page.locator('#error-text').text_content()
  assert await worker.evaluate('engine.combatPower.candidates.includes(self.fcCandidate)')
  result=await worker.evaluate('(()=>{const p=engine.combatPower.calculate(self.fcCandidate);return {current:p.current,projected:p.projected,valid:p.valid,gold:engine.call(engine.exports.get("getMyGold"))}})()')
  assert result=={'current':488,'projected':608,'valid':True,'gold':20000},result
  await page.screenshot(path='/tmp/destinia09-browser-fc-inventory.png')
  assert not errors,errors
  print(json.dumps({'result':'PASS','ui':75,'FC':result,'errors':errors}))
  await browser.close()
if __name__=='__main__':
 server=ThreadingHTTPServer(('127.0.0.1',0),functools.partial(Quiet,directory=str(ROOT/'dist')))
 threading.Thread(target=server.serve_forever,daemon=True).start()
 try:asyncio.run(run(f'http://127.0.0.1:{server.server_port}'))
 finally:server.shutdown();server.server_close()
