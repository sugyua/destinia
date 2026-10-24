import asyncio,json,os
from pathlib import Path
from playwright.async_api import async_playwright
async def main():
 async with async_playwright() as p:
  browser=await p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox','--disable-dev-shm-usage'])
  context=await browser.new_context(viewport={'width':430,'height':932},device_scale_factor=2,is_mobile=True,has_touch=True)
  page=await context.new_page();errors=[];page.on('pageerror',lambda error:errors.append(str(error)))
  await page.goto('http://127.0.0.1:8877/',wait_until='networkidle')
  await page.screenshot(path='/tmp/destinia-web-browser-welcome.png')
  assert await page.locator('#language').input_value()=='pt'
  assert await page.locator('#ui-scale').count()==0
  await page.locator('#language').select_option(os.environ.get('DESTINIA_LANGUAGE','pt'))
  await page.get_by_role('button',name='Iniciar teste').tap()
  await page.wait_for_function("Number(document.getElementById('game').dataset.frame)>100",timeout=45000)
  await page.locator('#drop-names').tap()
  worker=page.workers[0]
  assert await worker.evaluate('engine.dropNames.enabled===true')
  await page.locator('#drop-names').tap()
  assert await worker.evaluate('engine.dropNames.enabled===false')
  await page.keyboard.down('v');await page.wait_for_timeout(100)
  assert await worker.evaluate('engine.dropNames.enabled===true')
  await page.keyboard.up('v');await page.wait_for_timeout(100)
  assert await worker.evaluate('engine.dropNames.enabled===false')
  toggle=page.locator('#virtual-controls')
  await toggle.tap()
  await page.wait_for_function("document.getElementById('virtual-controls').getAttribute('aria-pressed')==='true'")
  worker=page.workers[0]
  assert await worker.evaluate("async()=>{for(let i=0;i<100;i++){if(engine.virtualControls===false)return true;await new Promise(r=>setTimeout(r,20));}return false;}")
  assert await page.locator('#controls').is_visible(),'Painel externo de toque deve permanecer'
  rect=await page.locator('#game').bounding_box()
  await page.touchscreen.tap(rect['x']+rect['width']*.5,rect['y']+rect['height']*.82)
  worker=page.workers[0]
  assert await worker.evaluate("async()=>{for(let i=0;i<200;i++){if(engine.menuState()===1)return true;await new Promise(r=>setTimeout(r,100));}return false;}"),'Toque nao abriu o menu principal'
  assert await worker.evaluate("async()=>{for(let i=0;i<200;i++){const b=engine.read(engine.gMenu+178,2);if((b[0]|b[1]<<8)>=16)return true;await new Promise(r=>setTimeout(r,100));}return false;}"),'Animacao do menu nao terminou'
  await page.screenshot(path='/tmp/destinia-web-browser-menu.png')
  await page.touchscreen.tap(rect['x']+rect['width']*.5,rect['y']+rect['height']*(120+(65-120)*min(100,100)/100)/240)
  assert await worker.evaluate("async()=>{for(let i=0;i<200;i++){if(engine.menuState()!==1)return true;await new Promise(r=>setTimeout(r,100));}return false;}"),'START nao mudou de tela'
  await page.screenshot(path='/tmp/destinia-web-browser-start.png')
  assert await page.locator('#error').is_hidden(),await page.locator('#error-text').text_content()
  await page.get_by_role('button',name='Pausar',exact=True).tap();await page.wait_for_function("document.getElementById('pause').getAttribute('aria-pressed')==='true'||!document.getElementById('error').hidden",timeout=10000);assert await page.locator('#error').is_hidden(),await page.locator('#error-text').text_content();before=await page.locator('#game').get_attribute('data-frame');await page.wait_for_timeout(400);after=await page.locator('#game').get_attribute('data-frame');assert before==after,(before,after)
  await page.get_by_role('button',name='Continuar',exact=True).last.tap();await page.wait_for_timeout(500)
  await page.set_viewport_size({'width':932,'height':430});await page.wait_for_timeout(500)
  await page.screenshot(path='/tmp/destinia-web-browser-landscape.png')
  overflow=await page.evaluate('document.documentElement.scrollWidth>innerWidth');assert not overflow,'Pagina excede largura no iPhone'
  assert not errors,errors
  assert await page.locator('#error').is_hidden(),await page.locator('#error-text').text_content()
  assert await page.evaluate("localStorage.getItem('destinia.virtualControls')")=='0'
  await page.reload(wait_until='networkidle')
  assert await page.locator('#virtual-controls').inner_text()=='Mostrar botões do jogo'
  assert await page.locator('#language').input_value()==os.environ.get('DESTINIA_LANGUAGE','pt')
  await page.get_by_role('button',name='Iniciar teste').tap()
  await page.wait_for_function("!document.getElementById('virtual-controls').disabled",timeout=30000)
  worker=page.workers[0]
  assert await worker.evaluate('engine.virtualControls===false')
  rect=await page.locator('#game').bounding_box()
  await page.mouse.move(rect['x']+rect['width']/2,rect['y']+rect['height']/2)
  await page.mouse.down()
  await page.wait_for_timeout(150)
  assert await worker.evaluate('engine.nativeTouchDown===true')
  await page.locator('#virtual-controls').tap()
  assert await worker.evaluate("async()=>{for(let i=0;i<100;i++){if(engine.virtualControls===true&&!engine.nativeTouchDown)return true;await new Promise(r=>setTimeout(r,20));}return false;}")
  await page.mouse.up()
  await worker.evaluate('engine.storeNotice=true')
  await page.locator('#store-notice').wait_for(state='visible')
  assert await worker.evaluate('paused===true')
  await page.locator('#close-store').tap()
  await page.wait_for_function("document.getElementById('store-notice').hidden")
  assert await worker.evaluate('paused===false')
  await worker.evaluate("engine.saved.set('save0.dat',new Uint8Array([7,8]))")
  page.once('dialog',lambda dialog:dialog.dismiss())
  await page.locator('#game-mode').select_option('expandido')
  assert await page.locator('#game-mode').input_value()=='classico'
  assert await worker.evaluate('engine.saved.size===1')
  page.once('dialog',lambda dialog:dialog.accept())
  await page.locator('#game-mode').select_option('expandido')
  await page.wait_for_function("!document.getElementById('pause').disabled",timeout=30000)
  worker=page.workers[0]
  assert await worker.evaluate('engine.gameMode==="expandido"&&engine.saved.size===0')
  assert await page.evaluate("localStorage.getItem('destinia.gameMode')")=="expandido"
  print(json.dumps({'result':'PASS','frames':await page.locator('#game').get_attribute('data-frame'),'pause_resume':True,'touch_menus':True,'native_state':await worker.evaluate('engine.menuState()'),'portrait_landscape':True,'virtual_controls_hidden':True,'external_touch_controls_retained':True,'preference_after_reload':True,'toggle_releases_active_gesture':True,'errors':errors}))
  await browser.close()
asyncio.run(main())
