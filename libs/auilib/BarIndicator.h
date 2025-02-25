#pragma once

#include <4dm.h>

using namespace fdm;

namespace aui {

	class BarIndicator : public gui::Element
	{
	public:
		
		bool isHorizontal = true;
		int width, height;
		int xOffset, yOffset;
		int fillDirection;
		bool clampFill = true;

		float maxFill=1;
		float currentFill=0.5;
		float fillFraction=0.5;
		glm::vec4 fillColor;
		glm::vec4 outlineColor;
		QuadRenderer* renderer = Item::qr;
		gui::Text text{};
		//std::string textFormatString = "{}"; no apparently
		bool showText = false;
		bool showMax = true;
		
		int textAlignment=0;

		void render(gui::Window* w) override
		{
			if (!isHorizontal) { // Vectical mode
				switch (fillDirection) {
				case 0: //down to up
					renderer->setPos(xOffset, height - height * fillFraction + yOffset, width, height * fillFraction);
					break;
				case 1: //center to the sides
					renderer->setPos(xOffset, height / 2 - height * fillFraction / 2 + yOffset, width, height * fillFraction);
					break;
				case 2: //up to down 
					renderer->setPos(xOffset, yOffset, width, height * fillFraction);
					break;
				}
				int textWidth, textHeight;
				text.getSize(w, &textWidth, &textHeight);
				switch (textAlignment) {
				case 0: //down
					text.offsetX(width / 2 - textWidth / 2 + xOffset);
					text.offsetY(height - textHeight / 2 + yOffset);
					break;
				case 1: //center
					text.offsetX(width / 2 - textWidth / 2 + xOffset);
					text.offsetY(height/2 - textHeight / 2 + yOffset);
					break;
				case 2: //up
					text.offsetX(width / 2 - textWidth / 2 + xOffset);
					text.offsetY(yOffset);
					break;
				}
				

				renderer->setQuadRendererMode(GL_TRIANGLES);
				renderer->setColor(fillColor.r, fillColor.g, fillColor.b, fillColor.a);
				renderer->render();

				renderer->setPos(xOffset, height / 2 + yOffset, width, height);
				renderer->setQuadRendererMode(GL_LINE_LOOP);
				renderer->setColor(outlineColor.r, outlineColor.g, outlineColor.b, outlineColor.a);
				renderer->render();

				text.render(w);
			}
			else  // Horizontal mode
			{
				switch (fillDirection) {
				case 0: //left to right
					renderer->setPos(xOffset ,yOffset, width * fillFraction, height );
					break;
				case 1: //center to the sides
					renderer->setPos(width/2 + xOffset - width * fillFraction/2,yOffset, width * fillFraction, height);
					break;
				case 2: //right to left
					renderer->setPos(width + xOffset - width * fillFraction,yOffset, width*fillFraction, height);
					break;
				}

				int textWidth, textHeight;
				text.getSize(w, &textWidth, &textHeight);
				textHeight = text.size * 7;
				switch (textAlignment) {
				case 0: //left
					text.offsetX( xOffset);
					text.offsetY(height/2 - textHeight / 2 + yOffset);
					break;
				case 1: //center
					text.offsetX(width / 2 - textWidth / 2 + xOffset);
					text.offsetY(height / 2 - textHeight / 2 + yOffset);
					break;
				case 2: //right
					text.offsetX(width - textWidth + xOffset);
					text.offsetY(height / 2 - textHeight / 2 + yOffset);
					break;
				}
				
				
				renderer->setQuadRendererMode(GL_TRIANGLES);
				renderer->setColor(fillColor.r, fillColor.g, fillColor.b, fillColor.a);
				renderer->render();

				renderer->setPos(xOffset,yOffset, width, height);
				renderer->setQuadRendererMode(GL_LINE_LOOP);
				renderer->setColor(outlineColor.r, outlineColor.g, outlineColor.b, outlineColor.a);
				renderer->render();
				
				text.render(w);
			}
		}

		inline void updateText() {
			std::string newText;
			if (showMax)
				newText = std::format("{}/{}", (int)std::ceil(currentFill), (int)std::ceil(maxFill));// ceil to avoid "0/max" output - will be "1/max" unitil death
			else
				newText = std::format("{}", (int)std::ceil(currentFill));
			text.setText(newText);
		}
		inline void clampFilling() {
			fillFraction = std::min(1.0f,std::max(fillFraction, 0.0f));
			currentFill = fillFraction * maxFill;
		}

		void setFillFraction(float fill) {
			fillFraction = fill;
			currentFill = fill* maxFill;
			updateText();
			if (clampFill) clampFilling();
		}
		void setFill(float fill) {
			currentFill = fill;
			fillFraction = fill / maxFill;
			updateText();
			if (clampFill) clampFilling();
		}
		void setMaxFill(float fill) {
			maxFill = fill;
			fillFraction = currentFill / maxFill;
			if (clampFill) clampFilling();
		}
		void setOutlineColor(glm::vec4 color) {
			outlineColor = color;
		}
		void setFillColor(glm::vec4 color) {
			fillColor = color;
		}

		void offsetX(int offset) override { xOffset = offset; }
		void offsetY(int offset) override { yOffset = offset; }
		void getSize(const gui::Window* w, int* x, int* y) const override { *x = width; *y = height; }
		void getPos(const gui::Window* w, int* x, int* y) const override { *x = xOffset; *y = yOffset; }
		void setSize(int width, int height) { this->width = width;this->height = height; }
	};
}