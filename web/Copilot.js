import React, { useState,useEffect } from 'react';
import "./copilot.css";

export default function Copilot() {
  const [count, setCount] = useState(0);
  const [hasToggleBeenClicked, setHasToggleBeenClicked] = useState(false);
  useEffect(() => {
    let toggleButton;
    setTimeout(() => {
      toggleButton = document.getElementById('toggle');
      toggleButton.addEventListener('click', () => {
        setHasToggleBeenClicked(true);
      });
    }, 100);
  }, []);

  return (
    <div id='copilotParent'>
      <p className="test">hi</p>
      {hasToggleBeenClicked && <p>Toggle has been clicked!</p>}
    </div>
  );
}