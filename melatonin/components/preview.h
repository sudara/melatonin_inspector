#pragma once

#include "melatonin_inspector/melatonin/component_model.h"

namespace melatonin
{

    class Preview : public juce::Component, public ComponentModel::Listener
    {
    public:
        int zoomScale = 20;
        juce::Rectangle<int> maxPreviewImageBounds;

        explicit Preview (ComponentModel& _model) : model (_model)
        {
            setInterceptsMouseClicks (true, true);
            model.addListener (*this);

#if MELATONIN_HAS_PAINT_DIAGNOSTICS
            addChildComponent (maxLabel);
            addChildComponent (avgLabel);
            addChildComponent (timingToggle);
            for (auto* l : { &maxLabel, &avgLabel })
            {
                l->setColour (juce::Label::textColourId, colors::iconOff);
                l->setJustificationType (juce::Justification::centredTop);
                l->setFont (InspectorLookAndFeel::getInspectorFont (18, juce::Font::FontStyleFlags::bold));
            }

            // by default timings aren't on
            timingToggle.on = settings->props->getBoolValue ("showPerformanceTimings", false);
            timingToggle.onClick = [this] {
                settings->props->setValue ("showPerformanceTimings", timingToggle.on);
                if (auto* parent = getParentComponent())
                    parent->resized();
            };
            timingToggle.setTooltip ("Show paint performance timings.\n\nDouble click the histograms to clear the history.");
#endif
        }

        ~Preview() override
        {
            model.removeListener (*this);
        }

        void paint (juce::Graphics& g) override
        {
            TRACE_COMPONENT();

            g.setColour (colors::black);
            g.fillRect (contentBounds);

#if MELATONIN_HAS_PAINT_DIAGNOSTICS
            if (showsPerformanceTimings())
            {
                maxLabel.setVisible (true);
                avgLabel.setVisible (true);

                // Tinted backdrops for the AVG/MAX value boxes.
                g.setColour (colors::propertyValueError.withAlpha (0.17f));
                g.fillRoundedRectangle (maxBounds.toFloat(), 3);
                g.setColour (colors::propertyName.withAlpha (0.10f));
                g.fillRoundedRectangle (avgBounds.toFloat(), 3);

                g.setFont (g.getCurrentFont().withHeight (15.0f));

                const auto& history = model.getPaintHistory();
                const bool hasExclusive = history.exclusive.filled > 0;
                const bool hasChildren = history.total.filled > 0;

                drawPerfRow (g, exclusiveBounds, "Exclusive", history.exclusive, hasExclusive);
                drawPerfRow (g, withChildrenBounds, "With Children", history.total, hasChildren);
            }
            else
            {
                maxLabel.setVisible (false);
                avgLabel.setVisible (false);
            }
#endif

            if (colorPicking)
            {
                // lets see them pixels!
                g.saveState();
                g.setImageResamplingQuality (juce::Graphics::ResamplingQuality::lowResamplingQuality);

                /* the zoomed snapshot is always *larger* than our preview area

                  bleed
                     │
                    ┌▼┌────────────────────┬─┐
                    │ │                   │ │
                    │ │                   │ │
                    │ │                   │ │
                    │ │                   │ │
                    └─┴────────────────────┴─┘
                 */
                int imageY = contentBounds.getY();
                int bleedPerSide = (previewImage.getWidth() * zoomScale - getWidth()) / 2;
                g.drawImageTransformed (previewImage, juce::AffineTransform::scale ((float) zoomScale, (float) zoomScale).translated ((float) -bleedPerSide, (float) imageY));

                // draw grid
                g.setColour (juce::Colours::grey.withAlpha (0.3f));
                for (auto i = 0; i < contentBounds.getHeight() / zoomScale; i++)
                    g.drawHorizontalLine (imageY + i * zoomScale, 0, (float) getWidth());

                int numberOfVerticalLines = previewImage.getWidth() - 1;
                auto inset = zoomScale - bleedPerSide;
                for (auto i = 0; i < numberOfVerticalLines; i++)
                    g.drawVerticalLine (inset + i * zoomScale, (float) imageY, (float) contentBounds.getBottom());

                // highlight the center pixel in first black, then white boxes
                g.setColour (juce::Colours::black);

                // grab the top left of the center pixel
                int highlightedPixelX = inset + (numberOfVerticalLines - 1) / 2 * zoomScale;
                int highlightY = (int) imageY + contentBounds.getHeight() / 2;
                g.drawRect (highlightedPixelX, highlightY - 10, zoomScale, zoomScale);
                g.setColour (juce::Colours::white);
                g.drawRect (highlightedPixelX - 2, highlightY - 12, 24, 24, 2);
                g.restoreState(); // back to full quality drawing
            }
            else if (!previewImage.isNull())
            {
                // TODO: odd this is needed (otherwise there's alpha in the state from somewhere)
                g.setOpacity (1.0f);

                // don't want our checkers aliased
                // so lets draw exact pixels
                g.saveState();
                g.setImageResamplingQuality (juce::Graphics::ResamplingQuality::lowResamplingQuality);

                // fits and scale the preview image and while doing so, grab the transform
                // this lets us reuse the position/scaling for clipping the transparency grid
                auto transform = juce::RectanglePlacement (juce::RectanglePlacement::centred).getTransformToFit (previewImage.getBounds().toFloat(), maxPreviewImageBounds.toFloat());
                auto resizedPreviewImageBounds = previewImage.getBounds().transformedBy (transform);

                // the transform is relative to maxPreviewImageBounds and has an "offset" already
                // For example, the Y offset will be at 48px from the Preview component
                // this would clip too much of the checkerboard (which is already placed at that offset).
                // An alternative solution here would be to have the checkerboard have the same size as Preview
                // but only start drawing the checkers at maxPreviewImageBounds
                auto checkersClipBounds = resizedPreviewImageBounds.translated(-maxPreviewImageBounds.getX(), -maxPreviewImageBounds.getY());

                // clipping keeps the checkerboard background fixed across image positions / sizes
                auto clippedCheckers = checkerboard.getClippedImage (checkersClipBounds);
                g.drawImage (clippedCheckers, resizedPreviewImageBounds.toFloat(), juce::RectanglePlacement::doNotResize);

                // back to drawing hi-res for the image
                g.restoreState();
                g.drawImageTransformed (previewImage, transform);
            }
        }

        void resized() override
        {
            TRACE_COMPONENT();

            auto area = getLocalBounds();
            buttonsBounds = area.removeFromTop (32);
#if MELATONIN_HAS_PAINT_DIAGNOSTICS
            timingToggle.setBounds (buttonsBounds.removeFromRight (32));
            buttonsBounds.removeFromRight (12);
#endif
            contentBounds = area;

#if MELATONIN_HAS_PAINT_DIAGNOSTICS
            if (showsPerformanceTimings())
            {
                auto performanceBounds = area.removeFromBottom (50).withLeft (32);

                // Two highlighted boxes spanning both rows on the right.
                // We carve them off the right with a small gap between so the
                // AVG and MAX columns read as separate sections, and a
                // breathing-room inset so MAX doesn't kiss the panel edge.
                constexpr int boxWidth = 80;
                constexpr int boxGap = 8;
                performanceBounds.removeFromRight (kBoxRightInset);
                maxBounds = performanceBounds.removeFromRight (boxWidth)
                                             .translated (0, -4).withTrimmedBottom (4);
                performanceBounds.removeFromRight (boxGap);
                avgBounds = performanceBounds.removeFromRight (boxWidth)
                                             .translated (0, -4).withTrimmedBottom (4);

                // Histogram area is whatever's between the row label and the
                // AVG box. We keep it as a single full-height region so paint()
                // can carve a per-row strip out of it.
                histogramArea = performanceBounds.withTrimmedLeft (100).withTrimmedRight (4);

                exclusiveBounds = performanceBounds.removeFromTop (25);
                withChildrenBounds = performanceBounds;

                auto positionRotatedLabel = [] (juce::Label& label, juce::Rectangle<int> b) {
                    auto pivot = b.getTopRight().toFloat();
                    label.setBounds (b.withLeft ((int) pivot.getX() - 50));
                    label.setTransform (juce::AffineTransform()
                        .rotated (-juce::MathConstants<float>::halfPi, pivot.getX(), pivot.getY())
                        .translated (-22, -2));
                };
                positionRotatedLabel (maxLabel, maxBounds);
                positionRotatedLabel (avgLabel, avgBounds);
            }
            else
            {
                exclusiveBounds = juce::Rectangle<int>();
                withChildrenBounds = juce::Rectangle<int>();
                histogramArea = juce::Rectangle<int>();
            }
#endif

            // default for this ends up being 32 48 382 68
            maxPreviewImageBounds = area.reduced (32, 16);
            drawCheckerboard();
        }

        void mouseDoubleClick (const juce::MouseEvent&) override
        {
#if MELATONIN_HAS_PAINT_DIAGNOSTICS
            if (model.getSelectedComponent())
            {
                model.clearPaintHistory();
                // force repaint to start re-collecting samples
                model.getSelectedComponent()->repaint();
                repaint();
            }
#endif
        }

        // called by color picker
        void setZoomedImage (const juce::Image& image)
        {
            previewImage = image;
            colorPicking = true;
            repaint();
        }

        void switchToPreview()
        {
            colorPicking = false;
            componentModelChanged (model);
            repaint();
        }

        [[nodiscard]] bool showsPerformanceTimings() const
        {
#if MELATONIN_HAS_PAINT_DIAGNOSTICS
            return !colorPicking && timingToggle.on && model.hasPaintHistory();
#else
            return false;
#endif
        }

    private:
        juce::Image previewImage;
        juce::Image checkerboard;
        juce::SharedResourcePointer<InspectorSettings> settings;
        ComponentModel& model;
        bool colorPicking = false;

        juce::Rectangle<int> buttonsBounds;
        juce::Rectangle<int> contentBounds;

#if MELATONIN_HAS_PAINT_DIAGNOSTICS
        // Layout bounds + UI elements that only exist when the JUCE paint
        // diagnostics callback is available. On older JUCE this entire panel
        // is hidden, so there's no reason to allocate the labels or toggle.
        juce::Rectangle<int> exclusiveBounds;
        juce::Rectangle<int> withChildrenBounds;
        juce::Rectangle<int> maxBounds;
        juce::Rectangle<int> avgBounds;
        juce::Rectangle<int> histogramArea;

        InspectorImageButton timingToggle { "timing", { 4, 4 }, true };
        juce::Label maxLabel { "max", "MAX" };
        juce::Label avgLabel { "avg", "AVG" };
#endif

        void componentModelChanged (ComponentModel&) override
        {
            TRACE_COMPONENT();

            if (auto component = model.getSelectedComponent())
                previewImage = component->createComponentSnapshot ({ component->getWidth(), component->getHeight() }, false, 2.0f);
            else
                previewImage = juce::Image();

            colorPicking = false;

#if MELATONIN_HAS_PAINT_DIAGNOSTICS
            updateTimingToggleVisibility();
#endif
        }

        void componentModelPaintHistoryUpdated (ComponentModel&) override
        {
#if MELATONIN_HAS_PAINT_DIAGNOSTICS
            updateTimingToggleVisibility();
#endif
            // Cheap path: no model rebuild, just redraw the timings overlay.
            if (showsPerformanceTimings())
                repaint();
        }

#if MELATONIN_HAS_PAINT_DIAGNOSTICS
        void updateTimingToggleVisibility()
        {
            const bool shouldBeVisible = model.hasPaintHistory();
            if (timingToggle.isVisible() == shouldBeVisible)
                return;

            timingToggle.setVisible (shouldBeVisible);
            if (auto* parent = getParentComponent())
                parent->resized();
        }

        // Histogram bar thresholds — the diagnostics callback captures every
        // paint at framerate, so the relevant scale is sub-millisecond.
        static constexpr double kHistogramYellowSec = 0.0003; // 0.3 ms
        static constexpr double kHistogramRedSec    = 0.001;  // 1.0 ms
        static constexpr double kHistogramFullScaleSec = 0.002; // 2 ms => full bar
        static constexpr int kHistogramBarWidth = 2;
        static constexpr int kHistogramBarGap   = 1; // separates back-to-back paints visually
        static constexpr int kHistogramBarPitch = kHistogramBarWidth + kHistogramBarGap;
        static constexpr int kHistogramBandHeight = 17; // sits inside the row, aligned with text
        static constexpr int kHistogramBandYOffset = -5; // raise bars to overlap the row text vertically
        static constexpr int kBoxValueLeftPad   = 2;  // breathing room before the value text
        static constexpr int kBoxValueRightPad  = 7;  // space between the value text and the rotated AVG/MAX label
        static constexpr int kBoxRightInset = 5; // breathing room between the MAX box and the panel edge

        void drawPerfRow (juce::Graphics& g,
                          juce::Rectangle<int> rowBounds,
                          const juce::String& rowLabel,
                          const PaintDiagnosticsHistory::Metric& metric,
                          bool hasData) const
        {
            g.setColour (hasData ? colors::propertyName : colors::propertyValueDisabled);
            g.drawText (rowLabel, rowBounds.withWidth (100), juce::Justification::topLeft);

            // Histogram band is vertically aligned with the row label (rather
            // than bottom-anchored to the row) so the bars read as part of the
            // same line of information as the row label and the AVG/MAX values.
            auto rowHistogram = histogramArea.withY (rowBounds.getY() + kHistogramBandYOffset)
                                             .withHeight (kHistogramBandHeight);
            drawHistogram (g, rowHistogram, metric);

            const auto avgValueBounds = avgBounds.withY (rowBounds.getY())
                                                 .withHeight (rowBounds.getHeight())
                                                 .withTrimmedLeft (kBoxValueLeftPad)
                                                 .withTrimmedRight (kBoxValueRightPad);
            const auto maxValueBounds = maxBounds.withY (rowBounds.getY())
                                                 .withHeight (rowBounds.getHeight())
                                                 .withTrimmedLeft (kBoxValueLeftPad)
                                                 .withTrimmedRight (kBoxValueRightPad);
            drawTimingText (g, avgValueBounds, metric.average(), !hasData);
            drawTimingText (g, maxValueBounds, metric.max, !hasData);
        }

        void drawHistogram (juce::Graphics& g,
                            juce::Rectangle<int> bounds,
                            const PaintDiagnosticsHistory::Metric& metric) const
        {
            if (metric.filled == 0 || bounds.isEmpty())
                return;

            // Histogram colours — explicit so they don't drift if the inspector
            // theme changes. Blue overrides timing-based colour to flag samples
            // where JUCE served the paint from the component's image cache,
            // since those are near-zero-cost paints regardless of the duration.
            static const juce::Colour green  = juce::Colour::fromRGB (98, 209, 116);
            static const juce::Colour yellow = juce::Colour::fromRGB (255, 204, 68);
            static const juce::Colour red    = colors::propertyValueError;
            static const juce::Colour blue   = juce::Colour::fromRGB (102, 187, 255);

            const int maxBars = juce::jmin (metric.filled, bounds.getWidth() / kHistogramBarPitch);
            const int skip = metric.filled - maxBars;
            const int oldestIdx = (metric.writeIdx - metric.filled + metric.capacity) % metric.capacity;

            for (int i = 0; i < maxBars; ++i)
            {
                const int sIdx = (oldestIdx + skip + i) % metric.capacity;
                const auto& sample = metric.samples[(size_t) sIdx];
                // refresh has real paint cost, so only true hits get blue
                const bool isCacheHit = (sample.cache == PaintDiagnosticsHistory::CacheState::hit);

                juce::Colour c;
                if (isCacheHit)
                    c = blue;
                else if (sample.seconds <= 0.0)
                    continue;
                else if (sample.seconds > kHistogramRedSec)
                    c = red;
                else if (sample.seconds > kHistogramYellowSec)
                    c = yellow;
                else
                    c = green;

                // hits register at ~zero — floor at 25% so the blue strip is visible
                const double scaled = isCacheHit
                    ? juce::jmax (0.25, juce::jmin (sample.seconds / kHistogramFullScaleSec, 1.0))
                    : juce::jmin (sample.seconds / kHistogramFullScaleSec, 1.0);
                const int barH = juce::jmax (1, (int) (scaled * (double) bounds.getHeight()));
                const int x = bounds.getX() + i * kHistogramBarPitch;
                const int y = bounds.getBottom() - barH;

                g.setColour (c);
                g.fillRect (x, y, kHistogramBarWidth, barH);
            }
        }

        static void drawTimingText (juce::Graphics& g, juce::Rectangle<int> bounds, double value, bool disabled = false)
        {
            auto text = timingWithUnits (disabled ? 0 : value);

            auto ms = value * 1000;
            if (disabled || ms * 1000 < 1)
                g.setColour (colors::propertyValueDisabled);
            else if (ms > 8)
                g.setColour (colors::propertyValueError);
            else if (ms > 3)
                g.setColour (colors::propertyValueWarn);
            else
                g.setColour (colors::propertyValue);

            g.drawText (text, bounds, juce::Justification::topLeft);
        }

        static juce::String timingWithUnits (double value)
        {
            double ms = value * 1000;
            if (ms * 1000 < 1)
                return "-";
            else if (ms < 1)
                return juce::String (ms * 1000, 1).dropLastCharacters (2) + juce::String (juce::CharPointer_UTF8 ("\xc2\xb5")) + "s"; // µs
            else
                return juce::String (ms, 1) + "ms";
        }
#endif

        // we draw the checkerboard at the full preview width and cache it
        // it's later clipped as needed
        void drawCheckerboard()
        {
            TRACE_COMPONENT();

            if (maxPreviewImageBounds.isEmpty())
                return;

            checkerboard = { juce::Image::RGB, maxPreviewImageBounds.getWidth(), maxPreviewImageBounds.getHeight(), true };
            juce::Graphics g2 (checkerboard);
            int checkerSize = settings->props->getIntValue ("checkerSize", 4);

            for (int i = 0; i < maxPreviewImageBounds.getWidth(); i += checkerSize)
            {
                // keeps checkerboard background consistent across image positions / sizes
                // allows for initial or ending partial checker
                for (auto j = 0; j < maxPreviewImageBounds.getHeight(); j += checkerSize)
                {
                    g2.setColour (((i + j) / checkerSize) % 2 == 0 ? colors::checkerLight : colors::checkerDark);
                    g2.fillRect (i, j, checkerSize, checkerSize);
                }
            }
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Preview)
    };
}
