/**
 * Gets a promise that resolves once a certain number of updates have happened
 * @param {number} threshold the value that the simulation's updates must reach
 * @returns {Promise} a promise that resolves once the number of updates in the current simulation is greater than or equal to the threshold value
 */
export default function getUpdatesThresholdPromise(threshold){
  return new Promise((resolve, reject) => {
    setTimeout(() => {
      let intervalId = setInterval(() => {
        let updatesSpan = document.getElementById('update');
        let updatesText = updatesSpan.innerText;
        
        let updates = parseInt(updatesText.split(' ')[2]);
        console.log(`updates: ${updates}`);
        if(updates >= threshold){
          clearInterval(intervalId);
          resolve();
        }
      }, 10);
    }, 100);
  });
}