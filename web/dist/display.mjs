// Automatic hiding is session-only; keep the user's saved touch preference.
export class TouchVisibility {
 constructor(preferred=true){this.preferred=preferred;this.controller=false;}
 get gameVisible(){return this.preferred&&!this.controller;}
 get siteVisible(){return !this.controller;}
 useGamepad(){this.controller=true;}
 disconnect(){this.controller=false;}
}
export async function toggleFullscreen(doc=document){
 if(doc.fullscreenElement){await doc.exitFullscreen();return 'normal';}
 if(doc.documentElement.requestFullscreen){await doc.documentElement.requestFullscreen();return 'fullscreen';}
 return doc.body.classList.toggle('expanded')?'expanded':'normal';
}
