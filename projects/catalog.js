'use strict';
(() => {
 const input=document.querySelector('#project-search');
 const cards=[...document.querySelectorAll('.project-card')];
 const sections=[...document.querySelectorAll('.project-section')];
 const filters=[...document.querySelectorAll('[data-filter]')];
 let category='all';
 const normalize=s=>s.toLocaleLowerCase('pl').normalize('NFD').replace(/[\u0300-\u036f]/g,'').replace(/ł/g,'l');
 function render(){
  const query=normalize(input.value.trim());let visible=0;
  for(const card of cards){const match=(category==='all'||card.dataset.category===category)&&normalize(card.dataset.search).includes(query);card.hidden=!match;if(match)visible++;}
  for(const section of sections){const count=[...section.querySelectorAll('.project-card')].filter(c=>!c.hidden).length;section.hidden=count===0;section.querySelector('.section-heading>span').textContent=`${count} / ${section.querySelectorAll('.project-card').length}`;}
  document.querySelector('#result-count').textContent=`${visible} z ${cards.length} projektów`;
  document.querySelector('.empty').hidden=visible!==0;
  for(const filter of filters)filter.setAttribute('aria-pressed',String(filter.dataset.filter===category));
 }
 input.addEventListener('input',render);
 for(const filter of filters)filter.addEventListener('click',()=>{category=filter.dataset.filter;render()});
 document.querySelector('#reset-search').addEventListener('click',()=>{input.value='';category='all';render();input.focus()});
 render();
})();
