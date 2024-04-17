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