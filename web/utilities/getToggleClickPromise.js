export default function getToggleClickPromise(){
  return new Promise((resolve, reject) => {
    setTimeout(() => {
      let toggleBtn = document.getElementById('toggle');
      
      function clickHandler(){
        toggleBtn.removeEventListener('click', clickHandler);
        resolve();
      };

      toggleBtn.addEventListener('click', clickHandler);
    }, 200);
  });
}