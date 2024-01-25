import React, { useState, useEffect } from 'react';
import CloseIcon from '@mui/icons-material/Close';

export default function CloseButton({ handleClose, copOpen }) {
  return (
    <>
      {
        copOpen &&
        <div id="copCloseBtnTrigger" className='fade-in'>
          <div id="copCloseBtn">
            <CloseIcon style={{ fontSize: 30 }} color='text' onClick={() => handleClose()} />
          </div>
        </div>
      }
      {
        !copOpen &&
        <div id="copCloseBtnTrigger" className='fade-out'>
          <div id="copCloseBtn">
            <CloseIcon style={{ fontSize: 30 }} color='text' onClick={() => handleClose()} />
          </div>
        </div>
      }
    </>


  );
}