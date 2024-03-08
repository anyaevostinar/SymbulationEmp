// emp__7
export default function getSynergyCheckPromise(goalValue){
    return new Promise((resolve, reject) => {
      setTimeout(() => {
        let intervalId = setInterval(() => {
          let synergyElement = document.getElementById('emp__27');
          //console.log(`updatesSpan: ${updatesSpan}`);
          let synergyVal = synergyElement.value;

          if(synergyVal == goalValue){
            clearInterval(intervalId);
            resolve();
          }
          else{
            //console.log(`updates not past threshold of ${threshold}`);
          }
        }, 10);
      }, 100);
    });
  }