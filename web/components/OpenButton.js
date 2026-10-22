/**
 * @name OpenButton
 * @description React component for the button that opens the CopilotWindow
 * @param {boolean} copOpen state in Copilot.js that determines if the Copilot window is open
 * @param {function} handleOpen function in Copilot.js that immediately sets copOpen to true and then sets displayOpenBtn to false and displayCopWindow to true after 150ms
 * @returns {JSX.Element} a div containing a button with a question mark icon that opens the Copilot window when clicked
 */
import React, { useState, useEffect } from 'react';
import "../copilot.css";
import QuestionMarkIcon from '@mui/icons-material/QuestionMark';

export default function OpenButton({ copOpen, handleOpen }) {
  const [btnClass, setBtnClass] = useState('');
  useEffect(() => {
    if (copOpen) {
      setBtnClass('fade-out');
    }
    else {
      setBtnClass('fade-in');
    }
    }, [copOpen])
  return (
    <div id="openBtnTrigger" onClick={() => handleOpen()}>
      <div id="openBtn">
        <QuestionMarkIcon style={{ fontSize: 32 }} color='text' className={btnClass} />
      </div>
    </div>
  );
}