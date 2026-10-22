/**
 * Gets a promise that resolves when the user clicks the "Reset" button
 * @returns {Promise} a promise that resolves when Symbulation's "Reset" button has been clicked
 */
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