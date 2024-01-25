export default function getToggleClickPromise(){
  return new Promise((resolve, reject) => {
    setTimeout(() => {
      let toggleBtn = document.getElementById('toggle');
      toggleBtn.addEventListener('click', () => {
        resolve();
      });
    }, 200);
  });
}