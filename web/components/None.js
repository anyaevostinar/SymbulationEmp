import React from "react";

export default function None({content}) {
  return(
    <div className="none">
      <h2 className="tutorialTitle">{content.titleText}</h2>
      <p className="tutorialDescription">{content.bodyText}</p>
    </div>
  );
}