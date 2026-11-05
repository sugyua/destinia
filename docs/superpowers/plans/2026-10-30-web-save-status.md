# Web save confirmation implementation plan

> Use superpowers:executing-plans. Owner authorized autonomous updates; this is the next Issue5 save-contract unit.

Goal: reliable Web feedback when the server does not confirm save/load, and no test access to personal saves.
Architecture: keep /api/save JSON and base64 payload unchanged. Async boolean acknowledgement in engine, atomic staging of downloaded files, optional notification callback to worker and a persistent page status. No PC/Drive server implementation or save-format change. Static VPN remains session-only.
Tech: JavaScript modules, Node test/http loopback ephemeral server, existing static Web.
Spec: Issue5 P1 save handling: reliable persistence warning, synthetic fixtures, preserve original saves.

Review focus: HTTPfailure even with okJSON, okfalse or malformedJSON/base64, partialload rollback, preserved feature sidecars, notification callback errors, worker/page status surviving FPS updates.

- [x] Replace legacy personal-data test with safe compatibility entrypoint; add web/tests/cloud-save.test.mjs with synthetic ephemeral HTTPserver. Run red before implementation.
- [x] Implement async confirmations + atomic load staging and status callback. Wire worker/page persistent status. Run directed tests and browser QA with real private assets but no personal saves.
- [x] Version0.17.2/docs/manifest; fullWeb/coherence/packages + Release/version/ARMnormalZoom with thermalguard68C.
- [ ] Independent review + finalCI; merge, release, downloads and local7entrypoints with25saves preserved; refresh privateVPN and central/checkpoint. Sites unavailable: do not retry or change audience.

Ledger:9RED before implementation,10GREEN for directed/legacy entries. Full197tests:196PASS/1SKIP; coherence6/package7/Windows2PASS; Release+ARM10framesnormalZoomPASS,CPU62.625/GPU45 under68C. Chromium390x844/844x390 realARM and persistent warningPASS. Independent reviews no blockers. Final CI and publication pending.
