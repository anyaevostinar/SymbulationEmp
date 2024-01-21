import React, { useState, useEffect } from 'react';
import "../copilot.css";
import QuestionMarkIcon from '@mui/icons-material/QuestionMark';

export default function OpenButton({ copOpen, handleOpen }) {
  const [btnClass, setBtnClass] = useState('');
  useEffect(() => {
    if (copOpen) {
      setBtnClass('fade-out');
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