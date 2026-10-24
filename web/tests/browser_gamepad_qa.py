"""Browser integration with an emulated Gamepad API. Does NOT validate USB/iOS."""
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
  await page.add_init_script("""window.padTest=null;window.inputTest=[];Object.defineProperty(navigator,'getGamepads',{value:()=>[window.padTest]});const post=Worker.prototype.postMessage;Worker.prototype.postMessage=function(m,...rest){if(['key','pause','touch'].includes(m.type))window.inputTest.push({...m});return post.call(this,m,...rest);};""")
  await page.goto(url);await page.locator('#start').tap();await page.wait_for_function('!document.getElementById("pause").disabled',timeout=45000)
  assert 'Nenhum controle' in await page.locator('#gamepad-status').inner_text()
  await page.evaluate("window.padTest={index:0,id:'Controle USB EMULADO',connected:true,mapping:'standard',axes:[0,0,0,0],buttons:Array.from({length:17},()=>({pressed:false,value:0}))}")
  await page.wait_for_function('document.getElementById("gamepad-status").textContent.includes("reconhecido")')
  await page.wait_for_timeout(100)
  await page.keyboard.down('ArrowRight');await page.evaluate('padTest.axes[0]=1;padTest.buttons[0].pressed=true;padTest.buttons[3].pressed=true')
  await page.wait_for_function('inputTest.some(e=>e.key===17&&e.down)&&inputTest.some(e=>e.key===27&&e.down)')
  await page.evaluate('padTest.axes[0]=0;padTest.buttons[0].pressed=false;padTest.buttons[3].pressed=false');await page.wait_for_timeout(150)
  assert not await page.evaluate('inputTest.some(e=>e.key===7&&!e.down)'),'pad released held keyboard alias'
  assert await page.locator('#controls').is_hidden()
  assert await page.workers[0].evaluate('engine.virtualControls===false')
  assert await page.workers[0].evaluate('inputs.held.has("key:7")')
  assert await page.evaluate('localStorage.getItem("destinia.virtualControls")') is None
  await page.locator('#fullscreen').click()
  await page.wait_for_function('!!document.fullscreenElement')
  assert await page.locator('#fullscreen').inner_text()=='Sair da tela cheia'
  assert not await page.evaluate('document.documentElement.scrollWidth>innerWidth')
  await page.locator('#fullscreen').click()
  await page.wait_for_function('!document.fullscreenElement')
  await page.keyboard.up('ArrowRight')
  await page.evaluate('padTest.buttons[9].pressed=true');await page.wait_for_function('!document.getElementById("paused").hidden')
  await page.wait_for_timeout(120);assert await page.evaluate('inputTest.filter(e=>e.type==="pause"&&e.value).length')==1
  await page.evaluate('padTest.buttons[9].pressed=false');await page.wait_for_timeout(100)
  await page.evaluate('padTest.buttons[9].pressed=true');await page.wait_for_function('document.getElementById("paused").hidden')
  await page.evaluate('padTest.buttons[9].pressed=false');await page.wait_for_timeout(100)
  await page.evaluate('padTest.axes[0]=1');await page.wait_for_timeout(100)
  await page.evaluate('window.dispatchEvent(new Event("blur"))');await page.wait_for_function('inputTest.at(-1).key===7&&inputTest.at(-1).down===false')
  old=await page.evaluate('inputTest.filter(e=>e.key===7&&e.down).length')
  await page.evaluate('window.dispatchEvent(new Event("focus"))');await page.wait_for_timeout(120)
  assert await page.evaluate('inputTest.filter(e=>e.key===7&&e.down).length')==old
  await page.evaluate('padTest.axes[0]=0');await page.wait_for_timeout(80)
  await page.evaluate('padTest.axes[0]=1');await page.wait_for_timeout(80)
  await page.evaluate('padTest=null');await page.wait_for_function('!document.getElementById("paused").hidden')
  assert await page.evaluate('inputTest.some(e=>e.key===7&&!e.down)')
  assert await page.locator('#controls').is_visible()
  assert await page.workers[0].evaluate('engine.virtualControls===true')
  await page.locator('#resume').tap();await page.locator('[data-key="17"]').tap()
  # Real touchscreen gesture remains independent of the emulated controller.
  rect=await page.locator('#game').bounding_box();await page.touchscreen.tap(rect['x']+rect['width']/2,rect['y']+rect['height']*.82)
  await page.wait_for_timeout(200)
  assert await page.evaluate('inputTest.some(e=>e.type==="touch"&&e.phase===0)')
  assert not errors,errors
  assert await page.locator('#error').is_hidden()
  print(json.dumps({'result':'PASS','emulated_gamepad_only':True,'usb_iphone':'PENDENTE','mapping_pause_focus_disconnect_keyboard_touch':True,'errors':errors}))
  fallback=await browser.new_page(viewport={'width':430,'height':932},is_mobile=True,has_touch=True)
  await fallback.add_init_script('Object.defineProperty(Element.prototype,"requestFullscreen",{value:undefined,configurable:true})')
  await fallback.goto(url)
  assert await fallback.locator('#fullscreen').inner_text()=='Ampliar tela'
  await fallback.locator('#fullscreen').tap()
  assert await fallback.evaluate('document.body.classList.contains("expanded")')
  assert 'barras do Safari' in await fallback.locator('#display-status').inner_text()
  await fallback.locator('#fullscreen').tap()
  assert not await fallback.evaluate('document.body.classList.contains("expanded")')
  await browser.close()
if __name__=='__main__':
 server=ThreadingHTTPServer(('127.0.0.1',0),functools.partial(Quiet,directory=str(ROOT/'dist')));threading.Thread(target=server.serve_forever,daemon=True).start()
 try:asyncio.run(run(f'http://127.0.0.1:{server.server_port}'))
 finally:server.shutdown();server.server_close()
