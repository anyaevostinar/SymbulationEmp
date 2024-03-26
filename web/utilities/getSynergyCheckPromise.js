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