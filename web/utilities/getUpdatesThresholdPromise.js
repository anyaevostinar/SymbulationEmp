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