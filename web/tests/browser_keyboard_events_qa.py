"""Browser/app input contract with an explicit worker fixture; not gameplay QA."""
import asyncio,os
from playwright.async_api import async_playwright

async def main():
 async with async_playwright() as p:
  options={'headless':True}
  if os.environ.get('DESTINIA_CHROMIUM'):options['executable_path']=os.environ['DESTINIA_CHROMIUM']
  browser=await p.chromium.launch(**options);page=await browser.new_page()
  await page.add_init_script("""window.testMessages=[];window.Worker=class {
   postMessage(m){testMessages.push(m);if(m.type==='start')queueMicrotask(()=>this.onmessage({data:{type:'ready'}}));}
   terminate(){}
  };""")
  await page.goto('http://127.0.0.1:8878/',wait_until='networkidle');await page.locator('#start').click()
  await page.wait_for_function("!document.getElementById('pause').disabled")
  await page.evaluate("window.keyDefaults=[];window.addEventListener('keydown',e=>queueMicrotask(()=>keyDefaults.push({key:e.key,prevented:e.defaultPrevented})),true)")
  for key,button in [('F4','weapons-catalog'),('F5','shared-stash'),('F8','bounties-btn')]:
   assert key in await page.locator('#'+button).text_content()
   await page.locator('#game').focus();await page.evaluate('testMessages.length=0')
   await page.keyboard.down(key);await page.keyboard.down(key);await page.keyboard.up(key)
   sent=await page.evaluate("testMessages.filter(m=>m.type==='key')")
   assert sent==[{'type':'key','key':key,'down':True},{'type':'key','key':key,'down':False}],sent
  for button,action in [('weapons-catalog','toggleWeaponsCatalog'),('shared-stash','toggleSharedStash'),('bounties-btn','toggleBounties')]:
   await page.locator('#'+button).click();assert await page.evaluate('testMessages.at(-1).type')==action
  await page.locator('#loot-settings').evaluate('(el)=>el.open=true')
  for field in ['loot-rank','language']:
   await page.locator('#'+field).focus();await page.keyboard.press('F5');await page.wait_for_timeout(100)
   assert not await page.locator('#pause').is_disabled(),'F5 reloaded active game in '+field
   assert await page.evaluate('keyDefaults.at(-1).prevented'),'F5 default not prevented in '+field
  await page.locator('#pause').click();await page.evaluate('testMessages.length=0')
  await page.locator('#loot-rank').focus()
  await page.keyboard.press('F5');await page.wait_for_timeout(100)
  assert await page.evaluate('keyDefaults.at(-1).prevented'),'F5 default not prevented in paused editor'
  assert await page.evaluate("testMessages.filter(m=>m.type==='key').length") == 0
  assert await page.locator('#pause').text_content()=='Continuar','F5 reloaded the paused app'
  await page.locator('#help').focus();await page.keyboard.press('Tab')
  assert await page.evaluate("document.activeElement.id!=='help'")
  print('BROWSER_INPUT_FIXTURE_OK: real app, F4/F5/F8 press/release/repeat, buttons, pause/F5, Tab; worker fixture, no gameplay')
  await browser.close()
asyncio.run(main())
