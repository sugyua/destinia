"""Real Chromium + app + worker + ARM; isolated browser context, no personal saves."""
import asyncio,os
from playwright.async_api import async_playwright

async def main():
 async with async_playwright() as p:
  options={'headless':True}
  if os.environ.get('DESTINIA_CHROMIUM'):options['executable_path']=os.environ['DESTINIA_CHROMIUM']
  browser=await p.chromium.launch(**options)
  page=await browser.new_page();errors=[]
  page.on('pageerror',lambda e:errors.append(str(e)))
  page.on('requestfailed',lambda r:print('REQUEST_FAILED',r.url,r.failure,flush=True))
  page.on('console',lambda m:print('CONSOLE',m.type,m.text[:200],flush=True) if m.type=='error' else None)
  await page.goto('http://127.0.0.1:8878/',wait_until='networkidle')
  await page.locator('#start').click()
  await page.wait_for_function("!document.getElementById('pause').disabled || !document.getElementById('error').hidden",timeout=60000)
  assert await page.locator('#error').is_hidden(),await page.locator('#error-text').text_content()
  worker=page.workers[0]
  for key,panel,button in [('F4','weaponsCatalog','weapons-catalog'),('F5','sharedStash','shared-stash'),('F8','bounties','bounties-btn')]:
   await page.locator('#game').focus()
   await page.keyboard.press(key);await page.wait_for_timeout(150)
   assert await worker.evaluate(f'engine.{panel}.open'),key+' did not open'
   await page.keyboard.press(key);await page.wait_for_timeout(150)
   assert not await worker.evaluate(f'engine.{panel}.open'),key+' did not close'
   await page.locator('#'+button).click();await page.wait_for_timeout(100)
   assert await worker.evaluate(f'engine.{panel}.open'),button
   await page.locator('#'+button).click();await page.wait_for_timeout(100)
   assert not await worker.evaluate(f'engine.{panel}.open'),button
  await page.locator('#pause').click();await page.keyboard.press('F5');await page.wait_for_timeout(200)
  assert len(page.workers)==1 and page.workers[0]==worker,'F5 reloaded paused game'
  assert not await worker.evaluate('engine.sharedStash.open'),'paused input reached game'
  await page.locator('#help').focus();await page.keyboard.press('Tab')
  assert await page.evaluate("document.activeElement.id!=='help'"),'Tab trapped'
  assert not errors,errors
  print('BROWSER_KEYBOARD_OK: F4/F5/F8 open-close, buttons, pause/F5, Tab; real app/worker/ARM')
  await browser.close()

asyncio.run(main())
