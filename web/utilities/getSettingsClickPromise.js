export default function getSettingsClickPromise(){
  return new Promise((resolve, reject) => {
    setTimeout(() => {
      let toggleBtn = document.getElementById('emp_settings');
      toggleBtn.addEventListener('click', () => {
        resolve();
      });
    }, 200);
  });
}