export const targetFPS=16;
// Preserve longer waits requested by ARM; never accelerate its update cadence.
export const frameDelay=(requested,elapsed)=>Math.max(0,Math.max(1000/targetFPS,requested)-elapsed);
