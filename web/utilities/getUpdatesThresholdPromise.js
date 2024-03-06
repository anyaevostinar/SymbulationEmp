export default function getUpdatesThresholdPromise(threshold){
  return new Promise((resolve, reject) => {
    setTimeout(() => {
      let intervalId = setInterval(() => {
        let updatesSpan = document.getElementById('update');
        console.log(`updatesSpan: ${updatesSpan}`);
        let updatesText = updatesSpan.innerText;
        console.log(`updatesText: ${updatesText}`);
        let updates = parseInt(updatesText.split(' ')[2]);
        console.log(`updates: ${updates}`);
        if(updates >= threshold){
          clearInterval(intervalId);
          console.log(`updates >= threshold of ${threshold}. resolving!`);
          resolve();
        }
        else{
          console.log(`updates not past threshold of ${threshold}`);
        }
      }, 10);
    }, 100);
  });
}