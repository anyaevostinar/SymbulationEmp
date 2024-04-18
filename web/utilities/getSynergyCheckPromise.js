/**
 * Gets a promises that resolves when the user sets synergy to a certain value
 * @param {number} goalValue the value that the synergy should be set to
 * @returns {Promise} a promise that resolves once the synergy value in Symbulation's settings has been set to the goalValue
 */
export default function getSynergyCheckPromise(goalValue){
    return new Promise((resolve, reject) => {
      setTimeout(() => {
        let intervalId = setInterval(() => {
          let synergyElement = document.getElementById('emp__27');
          let synergyVal = synergyElement.value;

          if(synergyVal == goalValue){
            clearInterval(intervalId);
            resolve();
          }
        }, 10);
      }, 100);
    });
  }