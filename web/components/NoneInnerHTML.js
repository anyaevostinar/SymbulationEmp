import React from "react";

export default function NoneInnerHTML({content}) {
  return(
    <div className="none">
      <h2 className="tutorialTitle">{content.titleText}</h2>
      <p className="tutorialDescription" dangerouslySetInnerHTML={{__html: content.bodyText}}></p>
    </div>
  );
}