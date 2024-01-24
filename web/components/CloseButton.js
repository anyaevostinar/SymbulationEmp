import React, { useState, useEffect } from 'react';
import CloseIcon from '@mui/icons-material/Close';

export default function CloseButton({ handleClose }) {
  return (
    <div id="copCloseBtnTrigger">
      <div id="copCloseBtn">
        <CloseIcon style={{ fontSize: 30 }} color='text' onClick={() => handleClose()} />
      </div>
    </div>

  );
}