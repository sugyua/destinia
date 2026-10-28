// Mirrors common/refinement_system.h. Pure policy; never writes original saves.
export const MaxLevel=15;
export const Stone=Object.freeze({Common:0,Pure:1,Abyssal:2});
function requireLevel(level){
  if(!Number.isInteger(level)||level<0||level>MaxLevel)throw new RangeError('invalid refinement level');
}
export function checkpoint(level){requireLevel(level);return level>=10?10:level>=5?5:0;}
export function stoneForTarget(target){
  requireLevel(target);if(target===0)throw new RangeError('refinement target must be positive');
  return target<=5?Stone.Common:target<=10?Stone.Pure:Stone.Abyssal;
}
export function stoneName(stone){
  if(!Number.isInteger(stone)||stone<0||stone>2)throw new RangeError('invalid refinement stone');
  return ['Pedra de Refino Comum','Pedra de Refino Pura','Pedra de Refino Abissal'][stone];
}
export function scale(base,level,percent){
  requireLevel(level);
  if(!Number.isSafeInteger(base)||base<0||!Number.isInteger(percent)||percent<0||percent>100)throw new RangeError('invalid refinement stat');
  const result=BigInt(base)+BigInt(base)*BigInt(level*percent)/100n;
  if(result>BigInt(Number.MAX_SAFE_INTEGER))throw new RangeError('refinement stat overflow');
  return Number(result);
}
export const weaponAttack=(base,level)=>scale(base,level,8);
export const armorDefense=(base,level)=>scale(base,level,6);
export const armorHealth=(base,level)=>scale(base,level,4);
export function quote(current){
  requireLevel(current);if(current===MaxLevel)throw new RangeError('maximum refinement reached');
  const target=current+1;
  return {target,stone:stoneForTarget(target),stones:target,gold:500*target*target,successPercent:100};
}
