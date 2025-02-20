#pragma once

#include <4dm.h>

using namespace fdm;

namespace aui {
	//Even more dogshit code, dont use it pls for your own sake
	class HBoxContainer : public gui::Element, public gui::ElemContainer
	{
	public:
		bool debug = false;
		std::vector<gui::Element*> elements{};
		QuadRenderer* background = Item::qr;
		uint32_t width = 100;
		uint32_t height = 100;

		unsigned int maxRows = 1;

		int xPos = 0;
		int yPos = 0;

		int xOffset = 0;
		int yOffset = 0;

		int elementXOffset = 0;
		int elementYOffset = 0;

		int xMargin = 10;
		int yMargin = 10;
		int xPadding = 8;
		int yPadding = 8;

		int xSpacing = 0;
		int ySpacing = 0;

		gui::AlignmentX xAlign = gui::ALIGN_LEFT;
		gui::AlignmentY yAlign = gui::ALIGN_TOP;

		bool renderBackground = false;

		void render(gui::Window* w) override
		{
			
			int Height = 0;
			int Width = 0;
			width = 0;
			height = 0;
			//Get size of the box based on child elements
			for (int i = 0;i < elements.size();i++) {
				elements[i]->getSize(w, &Width, &Height);

				//All characters in the font are 1 pixel higher, so have to manually calculate height of a text
				//Hope 0.3 font system will fix that 
				if (auto* text = dynamic_cast<gui::Text*>(elements[i]))
					Height = text->size * 7;

				if (Height > height) height = Height;
				width += Width+ xSpacing;
			}
			//Set position of the box
			w->getSize(&Width, &Height);
			width += xMargin*2;
			height += yMargin * 2;

			int rows = std::min(Height / (height + ySpacing), maxRows);
			int columns = (elements.size() + rows - 1) / rows;
			switch (xAlign) {
			case gui::ALIGN_CENTER_X:
				xPos = (Width / 2 - width / 2);
				break;
			case gui::ALIGN_LEFT:
				xPos = 0;
				break;
			case gui::ALIGN_RIGHT:
				xPos = (Width - width-xPadding );
				break;
			};

			switch (yAlign) {
			case gui::ALIGN_CENTER_Y:
				yPos = (Height / 2 - height / 2 );
				break;
			case gui::ALIGN_TOP:
				yPos = 0;
				break;
			case gui::ALIGN_BOTTOM:
				yPos = (Height - height-yPadding);
				break;
			};

			if (debug) {
				Console::printLine(std::format("offsetX: {}, pos {}, padding: {}", xOffset, xPos, xPadding));
				Console::printLine(std::format("offsetY: {}, pos {}, padding: {}", yOffset, yPos, yPadding));
			}

			//Arrange child elements
			int posY = yPos + elementYOffset + yMargin - (rows - 1) * (height / 2 + ySpacing)+yOffset;
			for (int j = 0;j < rows;j++) {
				int posX = xPos + elementXOffset + xMargin + xOffset;
				for (int i = 0;i < columns;i++) {
					int index = i * rows + j;
					if (index >= elements.size()) break;
					elements[index]->getSize(w, &Width, &Height);

					if (auto* text = dynamic_cast<gui::Text*>(elements[i]))
						Height = text->size * 7;

					elements[index]->offsetY(posY);
					//elements[i]->offsetX(posX + width / 2 - Width / 2); //alligns all elements to right
					elements[index]->offsetX(posX); //alligns all elements to center
					//elements[i]->offsetX(posX - width / 2 + Width / 2); //alligns all elements to left

					posX += Width + xSpacing;
				}
				posY += height + ySpacing;
			}

			
				//Render stuff
			if (renderBackground) {
				background->setPos(xPos+xOffset + xPadding, yPos + yOffset + yPadding, width - xPadding * 2, height - yPadding * 2);
				background->setQuadRendererMode(GL_TRIANGLES);
				background->setColor(0, 0, 0, 0.5);
				background->render();

				background->setQuadRendererMode(GL_LINE_LOOP);
				background->setColor(1, 1, 1, 0.5);
				background->render();
			}

			for (int i = 0;i < elements.size();i++) {

				elements[i]->render(w);
			}

		}

		void alignX(gui::AlignmentX alignment) override { xAlign = alignment; }
		void alignY(gui::AlignmentY alignment) override { yAlign = alignment; }
		void offsetX(int offset) override { xOffset = offset; }
		void offsetY(int offset) override { yOffset = offset; }
		void getSize(const gui::Window* w, int* x, int* y) const override { *x = width; *y = height; }
		void getPos(const gui::Window* w, int* x, int* y) const override { *x = xPos; *y = yPos; }

		void addElement(gui::Element* e) override
		{
			elements.push_back(e);
		}
		bool removeElement(gui::Element* e) override
		{
			return elements.erase(std::remove(elements.begin(), elements.end(), e), elements.end()) != elements.end();
		}
		void clear() override
		{
			elements.clear();
		}
		bool empty() override
		{
			return elements.empty();
		}

	};
}