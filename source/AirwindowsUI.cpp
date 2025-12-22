// AirwindowsUI by Chris Johnson
// Initial seed code for the meter created by Paul Walker on 8/23/21.
// From then on all this wild stuff is Chris :)
//and huge support from Sudara and Pamplejuce!
#include "AirwindowsUI.h"

void AirwindowsLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style, juce::Slider& slider) {
    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();
    float bevelW = sqrt((float)width);
    if (slider.isHorizontal()) bevelW = sqrt((float)height);
    float lineW = sqrt(bevelW)*0.55f;
    float trackWidth = bevelW;
    //basic variables we'll be using for our controls
    
    juce::Path backgroundTrack;
    g.setColour (findColour(juce::ResizableWindow::backgroundColourId).interpolatedWith (juce::Colours::white, 0.75f)); //highlight
    backgroundTrack.startNewSubPath((slider.isHorizontal()?(float)x:(float)x+(float)width*0.5f)+(lineW*0.5f), (slider.isHorizontal()?(float)y+(float)height*0.5f:(float)((height*0.97f)+y))+(lineW*0.5f));
    backgroundTrack.lineTo ((slider.isHorizontal()?(float)(width+x):(float)x+(float)width*0.5f)+(lineW*0.5f), (slider.isHorizontal()?(float)y+(float)height*0.5f:(float)y)+(lineW*0.5f));
    g.strokePath (backgroundTrack, {trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });
    backgroundTrack.clear();
    g.setColour (findColour(juce::ResizableWindow::backgroundColourId).interpolatedWith (juce::Colours::black, 0.75f)); //shadow
    backgroundTrack.startNewSubPath((slider.isHorizontal()?(float)x:(float)x+(float)width*0.5f)-lineW, (slider.isHorizontal()?(float)y+(float)height*0.5f:(float)((height*0.97f)+y))-lineW);
    backgroundTrack.lineTo ((slider.isHorizontal()?(float)(width+x):(float)x+(float)width*0.5f)-lineW, (slider.isHorizontal()?(float)y+(float)height*0.5f:(float)y)-lineW);
    g.strokePath (backgroundTrack, {trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });
    backgroundTrack.clear();
    g.setColour (findColour(juce::ResizableWindow::backgroundColourId)); //inside slot in which the slider goes
    backgroundTrack.startNewSubPath((slider.isHorizontal()?(float)x:(float)x+(float)width*0.5f), (slider.isHorizontal()?(float)y+(float)height*0.5f:(float)((height*0.97f)+y)));
    backgroundTrack.lineTo ((slider.isHorizontal()?(float)(width+x):(float)x+(float)width*0.5f), (slider.isHorizontal()?(float)y+(float)height*0.5f:(float)y));
    g.strokePath (backgroundTrack, {trackWidth*0.618f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });
    //draw the slot from which the slider moves. Note that we leave a bit of space on the bottom to show the label:
    
    g.setFont(juce::FontOptions(newFont, g.getCurrentFont().getHeight(), 0));
    g.setFont ((((lineW+bevelW)*24.0f) / (float)g.getCurrentFont().getHeight()));
    if (slider.isHorizontal()) bounds.removeFromBottom((bounds.getHeight()*0.5f)-(bevelW*3.2f));
    else bounds.removeFromBottom(-30.0f);
    g.setColour (findColour(juce::ResizableWindow::backgroundColourId).interpolatedWith (juce::Colours::white, 0.75f)); //highlight
    g.drawFittedText(slider.getName(), juce::Rectangle<int>((int)(bounds.getWidth()+23+lineW),(int)(bounds.getHeight()+lineW)), juce::Justification::centredBottom, 1);
    g.setColour (findColour(juce::ResizableWindow::backgroundColourId).interpolatedWith (juce::Colours::black, 0.75f)); //shadow
    g.drawFittedText(slider.getName(), juce::Rectangle<int>((int)(bounds.getWidth()+23-lineW),(int)(bounds.getHeight()-lineW)), juce::Justification::centredBottom, 1);
    g.setColour (findColour(juce::ResizableWindow::backgroundColourId).interpolatedWith (juce::Colours::black, 0.25f)); //text inside emboss
    g.drawFittedText(slider.getName(), juce::Rectangle<int>((int)bounds.getWidth()+23,(int)bounds.getHeight()), juce::Justification::centredBottom, 1);
    //This is the drawing of the text under the slider, to allow the slider to obscure it. Sliders are designed to be packed pretty tightly,
    //but the horizontal ones can still have a lot of text. To control their bulk, narrow the slot they're in.

    juce::Point<float> maxPoint = {slider.isHorizontal()?(sliderPos*0.91f)+(width*0.06f):((float)x+(float)width*0.5f), slider.isHorizontal()?((float)y+(float)height*0.5f):(sliderPos*0.91f)+(height*0.06f)};
    auto thumbWidth = (pow(bevelW,1.72f)*0.83f);
    auto rectSlider = juce::Rectangle<float>(thumbWidth*1.12f, thumbWidth*1.02f).withCentre(maxPoint);
    if (slider.isHorizontal()) rectSlider = juce::Rectangle<float>(thumbWidth*1.04f, thumbWidth*1.14f).withCentre(maxPoint);
    g.setColour (findColour(juce::ResizableWindow::backgroundColourId)); g.setOpacity(1.0f); g.fillRoundedRectangle (rectSlider, bevelW);
    //solid background for knob so you can't see the track under it
    juce::ColourGradient cg = juce::ColourGradient(juce::Colours::white, rectSlider.getTopLeft(), juce::Colours::black, rectSlider.getBottomRight(),false);
    cg.addColour(0.2f, juce::Colours::white); cg.addColour(0.618f, juce::Colours::transparentBlack); cg.addColour(0.9f, juce::Colours::black); cg.isRadial = true;
    g.setGradientFill(cg);
    auto inset = rectSlider; inset.reduce(bevelW*0.25f, bevelW*0.25f);
    g.drawRoundedRectangle (inset, bevelW*0.8f, bevelW*0.5f);
    cg = juce::ColourGradient(juce::Colours::transparentWhite, rectSlider.getTopLeft(), juce::Colours::black, rectSlider.getBottomRight(),false);
    cg.addColour(0.0955f, juce::Colours::white); cg.addColour(0.382f, slider.findColour (juce::ResizableWindow::backgroundColourId)); cg.addColour(0.618f, slider.findColour (juce::ResizableWindow::backgroundColourId)); cg.isRadial = true;
    g.setGradientFill(cg); inset.reduce(bevelW*0.25f, bevelW*0.25f); g.drawRoundedRectangle (inset, bevelW*0.9f, bevelW*0.382f);
    cg = juce::ColourGradient(juce::Colours::transparentWhite, rectSlider.getTopLeft(), juce::Colours::transparentBlack, rectSlider.getBottomRight(),false);
    cg.addColour(0.04775f, juce::Colours::transparentWhite); cg.addColour(0.382f, slider.findColour (juce::ResizableWindow::backgroundColourId)); cg.addColour(0.618f, slider.findColour (juce::ResizableWindow::backgroundColourId)); cg.isRadial = true;
    g.setGradientFill(cg); inset.reduce(bevelW*0.382f, bevelW*0.382f); g.drawRoundedRectangle (inset, 1.0f, 0.618f);
    g.setColour (juce::Colours::black); g.drawRoundedRectangle (rectSlider, bevelW, lineW);
    //This is the outside area of the slider knob, with the shading/highlighting that renders the 3D effect.
    
    float thumbScale = 0.85f; rectSlider = juce::Rectangle<float> (thumbWidth*thumbScale, thumbWidth*thumbScale).withCentre (maxPoint);
    rectSlider = juce::Rectangle<float>(thumbWidth*thumbScale, thumbWidth*thumbScale).withCentre(maxPoint);
    g.setColour (slider.findColour (juce::Slider::thumbColourId)); g.fillEllipse (rectSlider);
    cg = juce::ColourGradient(juce::Colours::white, rectSlider.getBottomRight(), juce::Colours::black, rectSlider.getTopLeft(),false);
    cg.addColour(0.191f, juce::Colours::white); cg.addColour(0.382f, slider.findColour (juce::Slider::thumbColourId)); cg.addColour(0.618f, slider.findColour (juce::Slider::thumbColourId)); cg.isRadial = true;
    g.setGradientFill(cg);
    inset = rectSlider; inset.reduce(bevelW*0.382f, bevelW*0.382f);
    g.drawEllipse (inset, bevelW*0.5f);
    cg = juce::ColourGradient(juce::Colours::white, rectSlider.getBottomRight(), juce::Colours::black, rectSlider.getTopLeft(),false);
    cg.addColour(0.0955f, juce::Colours::transparentWhite); cg.addColour(0.382f, slider.findColour (juce::Slider::thumbColourId)); cg.addColour(0.618f, slider.findColour (juce::Slider::thumbColourId)); cg.isRadial = true;
    g.setGradientFill(cg);
    inset.reduce(bevelW*0.125f, bevelW*0.125f);
    g.drawEllipse (inset, bevelW*0.5f); g.setColour (juce::Colours::black); g.drawEllipse (rectSlider, lineW);
    //This is the thumb of the knob, allowing a custom color to the thumb
}


void AirwindowsLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) {
    float tilt = (slider.findColour (juce::Slider::backgroundColourId).getFloatRed()-0.5f)*1.236f;
    auto textbounds = juce::Rectangle<int> (x, y, width, height).toFloat();
    auto bounds = textbounds; bounds.removeFromBottom(4.0f+(height*0.12f)+(height*tilt*tilt*0.12f)+(height*tilt*fabs(tilt)*0.12f)); bounds.reduce(1.0f, 1.0f);
    float scaleHeight = 1.0f-(fabs(tilt)); //proportion of vertical height relative to horizontal
    float trimscaleHeight = scaleHeight + (tilt*0.05f);
    float radius = bounds.getWidth()*0.5f; if (radius > (bounds.getHeight()/scaleHeight)*0.5f) radius = (bounds.getHeight()/scaleHeight)*0.5f;
    auto gradientSquare = juce::Rectangle<float>((float)bounds.getCentreX()-radius, (float)bounds.getCentreY()-(radius*(float)sqrt(scaleHeight)), radius*2.0f, radius*(float)sqrt(trimscaleHeight)*2.0f).toFloat();
    auto square = juce::Rectangle<float>((float)bounds.getCentreX()-radius, (float)bounds.getCentreY()-(radius*scaleHeight), radius*2.0f, radius*trimscaleHeight*2.0f).toFloat();
    float toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    float bevelW = (float)sqrt(radius*0.5f)*1.618f;
    float lineW = (float)sqrt(bevelW)*0.55f;
    //basic variables we'll be using for our controls
    
    juce::ColourGradient cg = juce::ColourGradient(juce::Colours::white, gradientSquare.getTopLeft(), juce::Colours::black, gradientSquare.getBottomRight(),false);
    cg.addColour(0.2f, juce::Colours::white); cg.addColour(0.618f, juce::Colours::transparentBlack); cg.addColour(0.9f, juce::Colours::black); cg.isRadial = true;
    g.setGradientFill(cg);
    auto inset = square; inset.reduce(bevelW*0.25f, bevelW*0.25f); inset.removeFromTop(-bevelW*tilt*fabs(tilt+0.15f)*0.618f);
    g.drawEllipse (inset, bevelW*0.5f);
    cg = juce::ColourGradient(juce::Colours::transparentWhite, gradientSquare.getTopLeft(), juce::Colours::black, gradientSquare.getBottomRight(),false);
    cg.addColour(0.0955f, juce::Colours::white); cg.addColour(0.382f, slider.findColour (juce::ResizableWindow::backgroundColourId)); cg.addColour(0.618f, slider.findColour (juce::ResizableWindow::backgroundColourId)); cg.isRadial = true;
    g.setGradientFill(cg);
    inset.reduce(bevelW*0.25f, bevelW*0.25f); inset.removeFromTop(-bevelW*tilt*fabs(tilt+0.15f)*0.618f);
    g.drawEllipse (inset, bevelW*0.382f);
    cg = juce::ColourGradient(juce::Colours::transparentWhite, gradientSquare.getTopLeft(), juce::Colours::transparentBlack, gradientSquare.getBottomRight(),false);
    cg.addColour(0.04775f, juce::Colours::transparentWhite); cg.addColour(0.382f, slider.findColour (juce::ResizableWindow::backgroundColourId)); cg.addColour(0.618f, slider.findColour (juce::ResizableWindow::backgroundColourId)); cg.isRadial = true;
    g.setGradientFill(cg);
    inset.reduce(bevelW*0.382f, bevelW*0.382f); inset.removeFromTop(-bevelW*tilt*fabs(tilt+0.15f)*0.618f);
    g.drawEllipse (inset, bevelW*0.618f); g.setColour (juce::Colours::black); g.drawEllipse (square, lineW);
    //This is the outside circle of the knob, with the shading/highlighting that renders the 3D effect. Tilting of the knob is included
    
    g.setFont(juce::FontOptions(newFont, g.getCurrentFont().getHeight(), 0));
    g.setFont ((((lineW+bevelW)*27.0f) / (float)g.getCurrentFont().getHeight()) + (tilt*0.25f));
    auto padHeight = (sqrt(bevelW)*-2.0f) + ((bounds.getHeight())-(bounds.getWidth()*scaleHeight)) + (tilt*bevelW) + (fabs(tilt)*-bevelW); if (padHeight < 0.0) padHeight = 0.0;
    textbounds.removeFromBottom(padHeight*0.618f);
    g.setColour (findColour(juce::ResizableWindow::backgroundColourId).interpolatedWith (juce::Colours::white, 0.75f)); //highlight
    g.drawFittedText(slider.getName(), juce::Rectangle<int>((int)(textbounds.getWidth()+lineW),(int)(textbounds.getHeight()+lineW)), juce::Justification::centredBottom, 1);
    g.setColour (findColour(juce::ResizableWindow::backgroundColourId).interpolatedWith (juce::Colours::black, 0.75f)); //shadow
    g.drawFittedText(slider.getName(), juce::Rectangle<int>((int)(textbounds.getWidth()-lineW),(int)(textbounds.getHeight()-lineW)), juce::Justification::centredBottom, 1);
    g.setColour (findColour(juce::ResizableWindow::backgroundColourId).interpolatedWith (juce::Colours::black, 0.25f)); //text inside emboss
    g.drawFittedText(slider.getName(), juce::Rectangle<int>((int)textbounds.getWidth(),(int)textbounds.getHeight()), juce::Justification::centredBottom, 1);
    //This is the drawing of the text just under the knob, no matter what tilt setting is included and what the real bounding box is doing. That's why it's so tricky :)

    auto arcRadius = (radius-(bevelW*0.5f))*0.7f;
    auto thumbWidth = pow(bevelW,1.618f)*0.7f;
    juce::Point<float> thumbPoint (bounds.getCentreX()+(arcRadius*std::cos(toAngle-juce::MathConstants<float>::halfPi)),bounds.getCentreY()+(-tilt*bevelW)+(arcRadius*trimscaleHeight*std::sin(toAngle-juce::MathConstants<float>::halfPi)));
    float thumbScale = 1.0f+(std::sin(toAngle-juce::MathConstants<float>::halfPi)*tilt*fabs(tilt)*0.5f);
    square = juce::Rectangle<float> (thumbWidth*thumbScale, thumbWidth*trimscaleHeight*thumbScale).withCentre (thumbPoint);
    gradientSquare = juce::Rectangle<float> (thumbWidth*trimscaleHeight*thumbScale, thumbWidth*thumbScale).withCentre (thumbPoint);
    g.setColour (slider.findColour (juce::Slider::thumbColourId)); g.fillEllipse (square);
    cg = juce::ColourGradient(juce::Colours::white, gradientSquare.getBottomRight(), juce::Colours::black, gradientSquare.getTopLeft(),false);
    cg.addColour(0.191f, juce::Colours::white); cg.addColour(0.382f, slider.findColour (juce::Slider::thumbColourId)); cg.addColour(0.618f, slider.findColour (juce::Slider::thumbColourId)); cg.isRadial = true;
    g.setGradientFill(cg);
    inset = square; inset.reduce(bevelW*0.382f, bevelW*0.382f); inset.removeFromBottom(bevelW*tilt*fabs(tilt+0.15f)*0.125f);
    g.drawEllipse (inset, bevelW*0.5f);
    cg = juce::ColourGradient(juce::Colours::white, gradientSquare.getBottomRight(), juce::Colours::black, gradientSquare.getTopLeft(),false);
    cg.addColour(0.0955f, juce::Colours::transparentWhite); cg.addColour(0.382f, slider.findColour (juce::Slider::thumbColourId)); cg.addColour(0.618f, slider.findColour (juce::Slider::thumbColourId)); cg.isRadial = true;
    g.setGradientFill(cg);
    inset.reduce(bevelW*0.125f, bevelW*0.125f); inset.removeFromBottom(bevelW*tilt*fabs(tilt+0.15f)*0.125f);
    g.drawEllipse (inset, bevelW*0.5f); g.setColour (juce::Colours::black); g.drawEllipse (square, lineW);
    //This is the thumb of the knob, also rendered on a tilt if needed, and allowing a custom color to the thumb
    
}

void AirwindowsMeter::paint(juce::Graphics &g)
{
    float vS = displayHeight/200.0f; // short for vScale: everything * this
    g.fillAll(juce::Colours::black);
    //blank screen before doing anything, unless our draw covers the whole display anyway
    //this is probably quick and optimized
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(0,  (int)(60.0f*vS), getWidth(),1); // -6dB markings
    g.fillRect(0, (int)(101.02*vS), getWidth(),1); //-12dB markings
    g.fillRect(0, (int)(130.02f*vS), getWidth(),1); //-18dB markings
    g.fillRect(0, (int)(150.2f*vS), getWidth(),1); //-24dB markings
    g.fillRect(0, (int)(164.9f*vS), getWidth(),1); //-30dB markings
    g.fillRect(0, (int)(175.2f*vS), getWidth(),1); //-36dB markings
    g.fillRect(0, (int)(182.5f*vS), getWidth(),1); //-42dB markings
    
    for (unsigned long count = 0; count < fmin(displayWidth,5150); ++count) //count through all the points in the array
    {
        g.setColour(juce::Colours::black);
        float psDotSizeL = 5.0f*dataA[count]*dataA[count];
        float psDotSizeR = 5.0f*dataB[count]*dataB[count];
        float peakL = dataC[count] * 200.0f;
        float peakR = dataD[count] * 200.0f;
        float slewL = sqrt(dataE[count])*300.0f;
        float slewR = sqrt(dataF[count])*300.0f;
        float meterZeroL = (sqrt(dataG[count])*6.0f)-6.0f;
        if (meterZeroL > 192.0f) meterZeroL = 192.0f;
        float meterZeroR = (sqrt(dataH[count])*6.0f)-6.0f;
        if (meterZeroR > 192.0f) meterZeroR = 192.0f;
        
        //begin draw dots on meters L
        if (peakL > 196.0f) {
            g.setColour(juce::Colour(255, 0, 0));
            g.fillRect((float)count, ((200.0f-fmin((peakL-196.0f),196.0f))*vS), 1.0f, (fmin((peakL-196.0f),196.0f)*vS));
        }//peak is clipping!
        g.setColour(juce::Colour::fromFloatRGBA(0.0f, 0.0f, psDotSizeL, 1.0f));
        g.fillRect((float)count, ((fmin((sqrt(meterZeroL)*14.142f)+26.0f,199.0f))*vS), ((dataA[count]*1.618f)+1.0f), fmax(8.0f*dataA[count]*vS,1.0f));
        //zero cross subs
        if (peakL > 1.0f && peakL < 196.0f) { //peak isn't clipping, but is not literally zero so there's something here to work with
            float departure = (slewL-peakL)*fabs(slewL-peakL);
            float hue = 0.25f+(departure*0.000073f);
            float saturation = 1.0f-fmax(hue-0.47f,0.0f);
            hue = fmin(hue,0.47f);
            if (hue < 0.0f) hue = fmax(hue-1.0f,0.69f);
            if (slewL > 196.0f) g.setColour(juce::Colours::white); //slew clip is rendered as brightest white
            else g.setColour(juce::Colour::fromHSV(hue, saturation, 1.0f, 1.0f));
            g.fillRect((float)count, ((200.0f - peakL)*vS), ((dataA[count]*1.618f)+1.0f), (fmax(fmax(psDotSizeL*vS,1.0f), dataC[count]*4.0f*vS)));
        } //end draw dots on meters L
        //begin draw dots on meters R
        if (peakR > 196.0f) {
            g.setColour(juce::Colour(255, 0, 0));
            g.fillRect((float)count, ((200.0f-fmin((peakR-196.0f),196.0f))*vS), 1.0f, (fmin((peakR-196.0f),196.0f)*vS));
        }//peak is clipping!
        g.setColour(juce::Colour::fromFloatRGBA(0.0f, 0.0f, psDotSizeR, 1.0f));
        g.fillRect((float)count, ((fmin((sqrt(meterZeroR)*14.142f)+26.0f,199.0f))*vS), ((dataB[count]*1.618f)+1.0f), fmax(8.0f*dataB[count]*vS,1.0f));
        //zero cross subs
        if (peakR > 1.0f && peakR < 196.0f) { //peak isn't clipping, but is not literally zero so there's something here to work with
            float departure = (slewR-peakR)*fabs(slewR-peakR);
            float hue = 0.25f+(departure*0.000073f);
            float saturation = 1.0f-fmax(hue-0.47f,0.0f);
            hue = fmin(hue,0.47f);
            if (hue < 0.0f) hue = fmax(hue-1.0f,0.69f);
            if (slewR > 196.0f) g.setColour(juce::Colours::white); //slew clip is rendered as brightest white
            else g.setColour(juce::Colour::fromHSV(hue, saturation, 1.0f, 1.0f));
            g.fillRect((float)count, ((200.0f - peakR)*vS), ((dataB[count]*1.618f)+1.0f), fmax(fmax(psDotSizeR*vS,1.0f), dataD[count]*4.0f*vS));
        } //end draw dots on meters R
    }
    g.setColour(juce::Colours::lightgrey);
    g.fillRect((int)dataPosition, 0, 1, (int)(200.0f*vS)); //the moving line
    float scaleFont = sqrt(vS*50.0f)*1.6f;
    if (scaleFont > 8.0f) {
        g.setColour(juce::Colour::fromHSV(0.0f, 0.0f, fmin(scaleFont-8.0f,0.7f), 1.0f));
        g.setFont(scaleFont);
        g.drawText("-6 dB", 0, (int)(60.0f*vS)-7, displayWidth-6, (int)scaleFont, juce::Justification::bottomRight);
        g.drawText("-12 dB", 0, (int)(101.02f*vS)-7, displayWidth-6, (int)scaleFont, juce::Justification::bottomRight);
        g.drawText("-18 dB", 0, (int)(130.02f*vS)-7, displayWidth-6, (int)scaleFont, juce::Justification::bottomRight);
        g.drawText("-24 dB", 0, (int)(150.2f*vS)-7, displayWidth-6, (int)scaleFont, juce::Justification::bottomRight);
        g.drawText("-30 dB", 0, (int)(164.9f*vS)-7, displayWidth-6, (int)scaleFont, juce::Justification::bottomRight);
        g.drawText("-36 dB", 0, (int)(175.2f*vS)-7, displayWidth-6, (int)scaleFont, juce::Justification::bottomRight);
        g.drawText("-42 dB", 0, (int)(182.5f*vS)-7, displayWidth-6, (int)scaleFont, juce::Justification::bottomRight);
        //right edge dB markings
        g.drawText("6.8k", 6, (int)(60.0f*vS)-7, displayWidth, (int)scaleFont, juce::Justification::bottomLeft);
        g.drawText("720", 6, (int)(101.02f*vS)-7, displayWidth, (int)scaleFont, juce::Justification::bottomLeft);
        g.drawText("230", 6, (int)(130.02f*vS)-7, displayWidth, (int)scaleFont, juce::Justification::bottomLeft);
        g.drawText("120", 6, (int)(150.2f*vS)-7, displayWidth, (int)scaleFont, juce::Justification::bottomLeft);
        g.drawText("80", 6, (int)(164.9f*vS)-7, displayWidth, (int)scaleFont, juce::Justification::bottomLeft);
        g.drawText("60", 6, (int)(175.2f*vS)-7, displayWidth, (int)scaleFont, juce::Justification::bottomLeft);
        g.drawText("50", 6, (int)(182.5f*vS)-7, displayWidth, (int)scaleFont, juce::Justification::bottomLeft);
        //left edge zero cross markings
    }
}
