/* Browser regression tests against mocked hardware responses, not the board. */
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const {chromium} = require(path.join(process.argv[2], 'playwright'));
const root = path.resolve(__dirname, '..');
const html = fs.readFileSync(path.join(root, 'web/index.html'), 'utf8');
const token = '0123456789abcdef0123456789abcdef';
(async () => {
  const browser = await chromium.launch({executablePath: 'C:/Program Files (x86)/Microsoft/Edge/Application/msedge.exe', headless: true});
  const page = await browser.newPage({viewport:{width:1024,height:1200}});
  const errors = [], posts = [];
  let scanFails = false;
  let statusSynced = false;
  let statusMode = 'portal';
  page.on('pageerror', error => errors.push(error.message));
  let config = {requestToken:token,ssid:'Test Network',hasPassword:true,ntpHost:'pool.ntp.org',ntpBackupHost:'time.cloudflare.com',
                timezone:'GMT0BST,M3.5.0/1,M10.5.0/2',syncHour:10,syncMinute:17,
                minuteHand:'gradual',secondHand:'continuous',nightParking:'off'};
  await page.route('http://clock.test/**', async route => {
    const req = route.request(), url = new URL(req.url());
    if (url.pathname === '/') return route.fulfill({contentType:'text/html',body:html});
    let data = {};
    if (req.method() === 'POST') {
      assert.equal(req.headers()['x-clock-token'], token);
      posts.push({path:url.pathname,body:Object.fromEntries(new URLSearchParams(req.postData()||''))});
      if (url.pathname === '/api/v1/dst-preview') {
        const rule = posts.at(-1).body.timezone;
        if (rule === 'bad') return route.fulfill({status:400,contentType:'application/json',body:JSON.stringify({error:'Enter a valid POSIX rule to preview DST.'})});
        const change = rule.startsWith('EST') ? {utc:1793512800,before:-240,after:-300} :
          rule.includes('M10.1.4') ? {utc:1790874000,before:0,after:60} :
          {utc:1792890000,before:60,after:0};
        return route.fulfill({contentType:'application/json',body:JSON.stringify({timeSynced:statusSynced,nextDst:statusSynced ? change : null})});
      }
      data = {saved:true,reset:true};
    } else if (url.pathname === '/api/v1/config') data = config;
    else if (url.pathname === '/api/v1/status') data = {mode:statusMode,ip:'192.168.4.1',firmwareVersion:'WiFi Clock · v2.0',buildId:'TXW813 HC32-V15 R16',hostname:'WiFi-Clock-ABCDEF',connected:false,timeSynced:statusSynced,timezone:'GMT0BST,M3.5.0/1,M10.5.0/2',nextDst:statusSynced ? {utc:1792890000,before:60,after:0} : null};
    else if (url.pathname === '/api/v1/scan') {
      if (scanFails) return route.fulfill({status:503,body:'Scan unavailable'});
      data = {scanning:false,networks:[
      {ssid:'Test Network',rssi:-65},{ssid:'Test Network',rssi:-40},{ssid:'manual',rssi:-60},
      {ssid:'<img src=x onerror=alert(1)>',rssi:-70}]};
    }
    else throw Error('Unimplemented frontend route '+url.pathname);
    await route.fulfill({contentType:'application/json',body:JSON.stringify(data)});
  });
  async function load() {
    await page.goto('http://clock.test/');
    await page.waitForFunction(() => !document.getElementById('sendTime').disabled &&
      !document.getElementById('scan').disabled && document.getElementById('networks').options.length >= 5);
  }
  await load();
  await page.waitForFunction(() => document.getElementById('firmwareVersion').title === 'TXW813 HC32-V15 R16');
  assert.equal(await page.locator('#firmwareVersion').textContent(),'WiFi Clock · v2.0');
  assert.equal(await page.locator('#projectLink').getAttribute('href'),'https://github.com/maddenste/Chouchin-899-latest-revision');
  assert.equal(await page.locator('#projectLink').textContent(),'WiFi Clock · v2.0 — Modified by Steve Madden · 2026');
  const footerColour = await page.locator('.firmwareVersion').evaluate(el => getComputedStyle(el).color);
  const linkColour = () => page.locator('#projectLink').evaluate(el => getComputedStyle(el).color);
  assert.equal(await linkColour(), footerColour);
  await page.locator('#projectLink').hover();
  assert.equal(await linkColour(), footerColour);
  await page.mouse.down();
  assert.equal(await linkColour(), footerColour);
  await page.mouse.move(0, 0);
  await page.mouse.up(); // Release away from link: do not open an external page.
  await page.locator('#projectLink').focus();
  assert.equal(await linkColour(), footerColour);
  await page.keyboard.press('Tab'); // Restore keyboard modality for focus-visible tests.
  assert.equal(await page.locator('#syncHour').inputValue(),'10');
  assert.equal(await page.locator('#syncMinute').inputValue(),'10');
  assert.match(await page.locator('#updateHelp').textContent(),/Saved time 10:17 is shown as 10:10/);
  assert.equal(await page.locator('#syncHour option').count(),24);
  assert.deepEqual(await page.locator('#syncMinute option').allTextContents(),['00','10','20','30','40','50']);
  assert.equal(await page.locator('#timezonePreset').inputValue(),'london');
  assert.equal(await page.locator('#nightParking').inputValue(),'off');
  assert.deepEqual(await page.locator('#nightParking option').allTextContents(),['Off','Night parking','On']);
  assert.match(await page.locator('label[for="nightParking"]').textContent(),/^Second hand battery saver/);
  assert.match(await page.locator('#parkingHelp').textContent(),/Use On if no second hand is fitted to prolong battery life/);
  assert.equal(await page.locator('#timezonePreset option').count(),40);
  assert.equal(await page.locator('#networks').inputValue(),'ssid:Test Network');
  assert.equal(await page.locator('#networks option:checked').textContent(),'Test Network');
  assert.equal(await page.locator('#networks option[value="ssid:Test Network"]').count(),1);
  assert.equal(await page.locator('#ssidRow').isVisible(),false);
  assert.equal(await page.locator('img').count(),0); // SSIDs cannot insert HTML.
  assert.deepEqual(await page.locator('.settingsGroup h2').allTextContents(),['Wi-Fi · 2.4 GHz','Time','Time servers','Hand movement']);
  assert.equal(await page.locator('#customTimezoneRow').isVisible(),false);
  assert.equal(await page.locator('#sendTime').textContent(),'Save settings');
  assert.equal(await page.locator('#factoryReset').isVisible(),false);
  assert.match(await page.locator('section.card').evaluate(el=>getComputedStyle(el).backdropFilter),/blur/);
  assert.match(await page.locator('header.card').evaluate(el=>getComputedStyle(el).backdropFilter),/blur/);
  assert.equal(await page.locator('header.card #mainTitle').count(),1);
  assert.equal(await page.locator('header.card #state').count(),1);
  await page.locator('#sendTime').focus();
  assert.notEqual(await page.locator('#sendTime').evaluate(el=>getComputedStyle(el).outlineStyle),'none');
  await page.locator('#sendTime').evaluate(el=>el.blur());
  await page.screenshot({path:path.join(root,'tests/web-desktop.png'),fullPage:true});
  await page.setViewportSize({width:390,height:844});
  statusSynced = true;
  await load();
  await page.waitForFunction(() => !document.getElementById('nextDst').classList.contains('hidden'));
  assert.equal(await page.locator('#nextDst').textContent(),'Next DST change: 25 October 2026 — 02:00 → 01:00');
  await page.selectOption('#dstMode','disabled');
  assert.equal(await page.locator('#nextDst').isVisible(),false);
  await page.selectOption('#dstMode','automatic');
  await page.waitForFunction(() => document.getElementById('nextDst').textContent.includes('25 October'));
  await page.selectOption('#timezonePreset','new_york');
  await page.waitForFunction(() => document.getElementById('nextDst').textContent.includes('1 November'));
  assert.match(await page.locator('#nextDst').textContent(),/02:00 → 01:00/);
  const savedPostsBefore = posts.filter(p=>p.path==='/api/v1/config').length;
  await page.selectOption('#dstMode','custom');
  await page.fill('#customTimezone','GMT0BST,M10.1.4/17:00,M11.1.0/2');
  await page.waitForFunction(() => document.getElementById('nextDst').textContent==='Next DST change: 1 October 2026 — 17:00 → 18:00');
  await page.fill('#customTimezone','bad');
  await page.waitForFunction(() => document.getElementById('nextDst').textContent.includes('valid POSIX'));
  await page.selectOption('#dstMode','disabled');
  assert.equal(await page.locator('#nextDst').isVisible(),false);
  assert.equal(posts.filter(p=>p.path==='/api/v1/config').length,savedPostsBefore);
  statusSynced = false;
  assert(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth));
  await page.screenshot({path:path.join(root,'tests/web-mobile.png'),fullPage:true});
  await page.setViewportSize({width:320,height:720});
  assert(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth));
  await page.emulateMedia({reducedMotion:'reduce'});
  assert.equal(await page.locator('#sendTime').evaluate(el=>getComputedStyle(el).transitionDuration),'0s');
  await page.emulateMedia({reducedMotion:'no-preference'});
  await page.setViewportSize({width:390,height:844});
  assert.equal(await page.locator('#handMovement option').allTextContents().then(labels=>labels.join('|')),'Sweep|Burst|Hold&Start');
  for (const [movement,minute,second] of [['sweep','gradual','continuous'],['burst','jump','continuous'],['hold_start','jump','pause_at_12']]) for (const parking of ['off','night','on']) {
    await load();
    await page.selectOption('#handMovement',movement); await page.selectOption('#nightParking',parking);
    assert.match(await page.locator('#movementHelp').textContent(),parking === 'on' ? /parks at 12/ : movement === 'hold_start' ? /pause at 12/ : movement === 'burst' ? /jumps/ : /gradually/);
    await page.click('#sendTime'); await page.waitForFunction(() => document.getElementById('notice').textContent.startsWith('Settings saved.'));
    const saved = posts.filter(p=>p.path==='/api/v1/config').at(-1).body;
    assert.equal(saved.minute_hand,minute); assert.equal(saved.second_hand,second); assert.equal(saved.night_parking,parking);
    assert.equal(saved.syncMinute,'10'); assert.equal(saved.password,'');
    assert.equal(saved.ssid,'Test Network');
    assert.equal(saved.ntpHost,''); assert.equal(saved.ntpBackupHost,'');
  }
  for (const mode of ['portal','station']) {
    statusMode = mode;
    await load();
    await page.click('#sendTime');
    await page.waitForFunction(() => document.getElementById('notice').textContent.startsWith('Settings saved.'));
    const message = await page.locator('#notice').textContent();
    assert.match(message, /send time once NTP synchronises/);
    assert.equal(message.includes('Rejoin your usual Wi-Fi network'), mode === 'portal');
  }
  statusMode = 'portal';
  for (const hour of ['00','02','23']) for (const minute of ['00','10','20','30','40','50']) {
    await load();
    await page.selectOption('#syncHour',hour); await page.selectOption('#syncMinute',minute);
    await page.click('#sendTime');
    await page.waitForFunction(() => document.getElementById('notice').textContent.startsWith('Settings saved.'));
    const saved = posts.filter(p=>p.path==='/api/v1/config').at(-1).body;
    assert.equal(saved.syncHour,hour); assert.equal(saved.syncMinute,minute);
  }
  for (const [movement,minute,second] of [['sweep','gradual','continuous'],['burst','jump','continuous'],['hold_start','jump','pause_at_12']]) {
    config = {...config,minuteHand:minute,secondHand:second};
    await load();
    assert.equal(await page.locator('#handMovement').inputValue(),movement);
  }
  config = {...config,minuteHand:'gradual',secondHand:'continuous'};
  await load(); await page.selectOption('#networks','ssid:manual'); assert.equal(await page.locator('#ssid').inputValue(),'manual');
  assert.equal(await page.locator('#ssidRow').isVisible(),false); // Actual SSID named manual.
  await page.selectOption('#networks','manual');
  assert.equal(await page.locator('#ssidRow').isVisible(),true);
  await page.fill('#ssid','Hidden Network');
  await page.click('#sendTime');
  await page.waitForFunction(() => document.getElementById('sendTime').disabled);
  assert.equal(posts.filter(p=>p.path==='/api/v1/config').at(-1).body.ssid,'Hidden Network');
  await load(); await page.selectOption('#networks','');
  assert.equal(await page.locator('#ssidRow').isVisible(),false);
  await page.click('#sendTime');
  assert.match(await page.locator('#notice').textContent(),/Choose a Wi-Fi network/);
  assert.equal(await page.locator('#networks').evaluate(el=>el===document.activeElement),true);
  await page.selectOption('#networks','manual');
  assert.equal(await page.locator('#ssidRow').isVisible(),true);
  await page.selectOption('#networks','ssid:Test Network');
  assert.equal(await page.locator('#ssidRow').isVisible(),false);
  await load();
  await page.selectOption('#dstMode','custom'); await page.fill('#customTimezone',''); await page.click('#sendTime');
  assert.equal(await page.locator('#customTimezoneRow').isVisible(),true);
  assert.match(await page.locator('#notice').textContent(),/Enter a custom/);
  await page.fill('#customTimezone','IST-5:30');
  await page.click('#sendTime'); await page.waitForFunction(() => document.getElementById('sendTime').disabled);
  assert.equal(posts.filter(p=>p.path==='/api/v1/config').at(-1).body.timezone,'IST-5:30');
  for (const id of ['ntpHost','ntpBackupHost']) {
    await load();
    await page.fill('#'+id,'https://bad.example');
    const before = posts.filter(p=>p.path==='/api/v1/config').length;
    await page.click('#sendTime');
    assert.match(await page.locator('#notice').textContent(),/NTP server/);
    assert.equal(posts.filter(p=>p.path==='/api/v1/config').length,before);
  }
  config = {...config,ntpHost:'primary.example.net',ntpBackupHost:'192.168.0.2'};
  await load();
  assert.equal(await page.locator('#ntpHost').inputValue(),'primary.example.net');
  assert.equal(await page.locator('#ntpBackupHost').inputValue(),'192.168.0.2');
  await page.fill('#ntpBackupHost',' backup.example.net ');
  await page.click('#sendTime');
  await page.waitForFunction(() => document.getElementById('sendTime').disabled);
  assert.equal(posts.filter(p=>p.path==='/api/v1/config').at(-1).body.ntpBackupHost,'backup.example.net');
  await load(); await page.click('#resetOptions summary'); page.once('dialog',d=>d.accept()); await page.click('#factoryReset');
  await page.waitForFunction(() => document.getElementById('notice').textContent.startsWith('Saved settings erased.'));
  assert(posts.some(p=>p.path==='/api/v1/factory-reset'));
  config = {...config,ssid:'Saved Hidden Network'};
  await load();
  assert.equal(await page.locator('#networks').inputValue(),'ssid:Saved Hidden Network');
  assert.equal(await page.locator('#networks option:checked').textContent(),'Saved Hidden Network');
  assert.equal(await page.locator('#ssidRow').isVisible(),false);
  assert.equal(await page.locator('#ssid').inputValue(),'Saved Hidden Network');
  await page.selectOption('#networks','manual');
  assert.equal(await page.locator('#ssidRow').isVisible(),true);
  scanFails = true;
  await page.goto('http://clock.test/');
  await page.waitForFunction(() => !document.getElementById('scan').disabled &&
    document.getElementById('notice').textContent.includes('Scan unavailable'));
  assert.equal(await page.locator('#networks').inputValue(),'ssid:Saved Hidden Network');
  assert.equal(await page.locator('#ssidRow').isVisible(),false);
  scanFails = false;
  config = {...config,ssid:'',hasPassword:false};
  await load();
  assert.equal(await page.locator('#networks').inputValue(),'');
  assert.equal(await page.locator('#networks option:checked').textContent(),'Select a network');
  assert.equal(await page.locator('#ssid').inputValue(),'');
  await page.selectOption('#networks','ssid:Test Network');
  await page.click('#scan');
  await page.waitForFunction(() => document.getElementById('notice').textContent.includes('network(s) found'));
  assert.equal(await page.locator('#networks').inputValue(),'ssid:Test Network');
  assert.deepEqual(errors,[]);
  await browser.close();
  console.log('Browser UI: desktop/mobile, 40 locations, movement/saving options, ten-minute update selection, explicit legacy-time rounding, saved SSID/scan behavior, dual NTP validation, password retention and reset passed');
})().catch(error => {console.error(error);process.exit(1)});
