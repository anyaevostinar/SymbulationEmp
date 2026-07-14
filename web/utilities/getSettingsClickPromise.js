/**
 * Gets a promise that resolves when the user opens/closes the settings card
 * The promise will resolve regardless of whether the settings card is opened or closed
 * @returns {Promise} a promise that resolves when Symbulation's settings card header is clicked
 */
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