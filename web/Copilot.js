import React, { useState } from 'react';
import "./copilot.css";

export default function Tutorial() {
  const [count, setCount] = useState(0);

  return (
    <div id='copilotParent'>
      <p className="test">hi</p>
    </div>
  );
}