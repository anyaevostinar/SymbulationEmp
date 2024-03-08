export default function getResetClickPromise() {
    return new Promise((resolve, reject) => {
        setTimeout(() => {
            let toggleBtn = document.getElementById('reset');
            toggleBtn.addEventListener('click', () => {
                resolve();
            });
        }, 200);
    });
}