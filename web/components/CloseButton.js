/**
 * @name CloseButton
 * @description React component for the Copilot window's closes button
 * @param {function} handleClose function in Copilot.js that immediately sets copOpen to false and then sets displayOpenBtn to true and displayCopWindow to false after 150ms
 * @param {boolean} copOpen state in Copilot.js that determine if the Copilot window is open
 * @returns {JSX.Element} a button with an "X" icon that closes Copilot when clicked
 */
import React, { useState, useEffect } from 'react';
import CloseIcon from '@mui/icons-material/Close';

export default function CloseButton({ handleClose, copOpen }) {
  return (
    <>
      {
        // If the Copilot window is open, display the close button with a fade-in animation
        copOpen &&
        <div id="copCloseBtnTrigger" className='fade-in'>
          <div id="copCloseBtn">
            <CloseIcon style={{ fontSize: 30 }} color='text' onClick={() => handleClose()} />
          </div>
        </div>
      }
      {
        // If the Copilot window is closed, display the close button with a fade-out animation
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