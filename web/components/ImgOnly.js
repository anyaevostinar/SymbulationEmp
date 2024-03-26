import React from "react";

export default function ImgOnly({content}) {
  return(
    <div className="imgOnly">
      <h2 className="tutorialTitle">{content.titleText}</h2>
      <p className="tutorialDescription">{content.bodyText}</p>
      <div className="imgHolder">
        <img src={content.imgSrc} className='tutorialImg' />
      </div>
    </div>
  );
}