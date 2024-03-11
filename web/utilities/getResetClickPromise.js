import { tutorialTrackerAtom } from "../atoms";


export default function getResetClickPromise() {
    return new Promise((resolve, reject) => {
        setTimeout(() => {
            let resetBtn = document.getElementById('reset');

            function clickHandler() {
                resetBtn.removeEventListener('click', clickHandler);
                resolve();
            }

            resetBtn.addEventListener('click', clickHandler);
        }, 200);
    });
}
/*
export default function getToggleClickPromise(){
  return new Promise((resolve, reject) => {
    setTimeout(() => {
      let toggleBtn = document.getElementById('toggle');
      
      let clickHandler = () => {
        toggleBtn.removeEventListener('click', clickHandler);
        resolve();
      };

      toggleBtn.addEventListener('click', clickHandler);
    }, 200);
  });
}
*/