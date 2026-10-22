/**
 * Gets a promise that resolves when the user clicks the toggle button (that displays either "Start" or Pause")
 * @returns {Promise} a promise that resolves when Symbulation's toggle button is clicked
 */
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