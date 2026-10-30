# Unicorn startup checks implementation plan

> For agentic workers: use superpowers:executing-plans. Owner authorized autonomous checkpoint updates; no new product decision is required.

Goal: stop both Windows hosts on essential Unicorn initialization errors before any save access.
Architecture: shared error formatter throws into existing main cleanup; check existing five maps and two core hooks. Move save preparation after core initialization. No memory addresses, hook semantics, ARM library or save formats change.
Tech stack: C++17, Unicorn, Python unittest, MSYS2/UCRT64.
Spec: Issue #5 P1 initialization; scoped to core engine/maps/stub/return hooks, not all optional feature hooks or a loader redesign.

## Constraints and review focus
- No personal saves in tests. Existing cleanup must close partial engine exactly once.
- Each of five map failures and two hook failures must stop before initGame and save creation/recovery.
- Existing saves and pending journals must remain byte-identical on rejected boot.
- Genuine compatible boot and both normal/Zoom paths remain successful.
- Failure injection exists only in test fixture, never distributed host.

## Task 1
- [x] Add fixture-only injection for open, each map and core hook; assert diagnostics/nonzero/engine cleanup/no save mutation.
- [x] Run open failure test on baseline: expect failure because save folder was created.
- [x] Add common/unicorn_check.h requireUnicorn(result,operation,address,size) and use for core calls in both hosts; postpone save setup until mandatory initialization succeeds.
- [x] Run full runtime suite plus builds/10 ARM frames under thermal guard68C.

## Task 2
- [x] Record Issue5/PR7/PR8 triage with actual code evidence and remaining work. Do not merge old PRs or close issue during analysis.
- [x] Version0.17.1, README/CHANGELOG/manifest/current notes/report; update Web version metadata only, document VPN status and unchanged game logic.
- [ ] Coherence/package/Node tests; fresh code review; final CI before merge.
- [ ] Verified release/download/install7entrypoints, save hashes preserved. Refresh private VPN static files. Central/checkpoint after delivery. No retry unavailable Sites.

Ledger: red failed both hosts; green15runtimePASS. Final local validation186NodePASS/1SKIP, coherence5/package7/Windows2PASS, Release+ARM10framesbothPASS. ThermalCPU63.25/GPU45. Independentreview: no blockers, minor second-call assertion coverage noted; first-call exhaustive checks and existing-save diagnostics preserve scoped confidence. CI and publication pending.
