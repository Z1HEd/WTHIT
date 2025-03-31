//#define DEBUG_CONSOLE // Uncomment this if you want a debug console to start. You can use the Console class to print. You can use Console::inStrings to get input.

#include <4dm.h>
#include "auilib/auilib.h"
#include "BetterUI.h"
#include <fstream> // actually i do need that for config reading apparently

using namespace fdm;

initDLL

aui::VBoxContainer settingsContainer;

QuadRenderer qr{};
FontRenderer font{};

gui::Interface ui;

aui::HBoxContainer tipContainer; // for the whole box
aui::VBoxContainer textContainer; // just for name and position
aui::HBoxContainer coordsContainer; // for seperate coordinates

gui::Image icon;
gui::Text nameText{};
aui::BarIndicator healthBar;
gui::Text positionTextX{}; // Seperate texts for each coord because i want them to be different colors like in compass
gui::Text positionTextY{}; // I want BBCode for cristmass Mashpoe pls
gui::Text positionTextZ{};
gui::Text positionTextW{};

glm::uint8_t targetBlockId;
std::string targetName;
glm::vec4 targetPos;
std::unordered_map<uint8_t, std::string> blockNames;

gui::AlignmentX alignmentX;
gui::AlignmentY alignmentY;

gui::Text title;
gui::Slider xSlider{};
gui::Slider ySlider{};



static bool initializedSettings = false;
static bool isTargeting = false;
static bool isTargetingBlock = false;
static float currentTargetHealth = -1;
static float currentTargetMaxHealth = -1;
static bool isDisplayingHealth = false;
static bool isDisplayingCoords = false;
std::string configPath;


std::string getBlockName(uint8_t blockId) {
	auto it = BlockInfo::blockNames.find(blockId);
	if (it != BlockInfo::blockNames.end()) {
		return it->second;
	}
	return "Unknown Block";
}

void viewportCallback(void* user, const glm::ivec4& pos, const glm::ivec2& scroll)
{
	GLFWwindow* window = (GLFWwindow*)user;

	// update the render viewport
	int wWidth, wHeight;
	glfwGetWindowSize(window, &wWidth, &wHeight);
	glViewport(pos.x, wHeight - pos.y - pos.w, pos.z, pos.w);

	// create a 2D projection matrix from the specified dimensions and scroll position
	glm::mat4 projection2D = glm::ortho(0.0f, (float)pos.z, (float)pos.w, 0.0f, -1.0f, 1.0f);
	projection2D = glm::translate(projection2D, { scroll.x, scroll.y, 0 });

	// update all 2D shaders
	const Shader* textShader = ShaderManager::get("textShader");
	textShader->use();
	glUniformMatrix4fv(glGetUniformLocation(textShader->id(), "P"), 1, GL_FALSE, &projection2D[0][0]);

	const Shader* tex2DShader = ShaderManager::get("tex2DShader");
	tex2DShader->use();
	glUniformMatrix4fv(glGetUniformLocation(tex2DShader->id(), "P"), 1, GL_FALSE, &projection2D[0][0]);

	const Shader* quadShader = ShaderManager::get("quadShader");
	quadShader->use();
	glUniformMatrix4fv(glGetUniformLocation(quadShader->id(), "P"), 1, GL_FALSE, &projection2D[0][0]);
}

//Initialize stuff when entering world
$hook(void, StateGame, init, StateManager& s)
{
	original(self, s);

	tipContainer.clear();
	textContainer.clear();
	coordsContainer.clear();

	font = { ResourceManager::get("pixelFont.png"), ShaderManager::get("textShader") };


	qr.shader = ShaderManager::get("quadShader");
	qr.init(); 

	ui = gui::Interface{ s.window };
	ui.viewportCallback = viewportCallback;
	ui.viewportUser = s.window;
	ui.font = &font;
	ui.qr = &qr;

	icon.tr = &ItemBlock::tr;
	icon.width = 60;
	icon.height = 62;

	nameText.text = "Target name";
	nameText.alignX(gui::ALIGN_LEFT);
	nameText.size = 2;
	nameText.shadow = true;
	
	healthBar.showFractionLines = true;
	healthBar.setSize(200,10);
	healthBar.setFillColor({ 154.0f / 256.0f, 20.0f / 256.0f, 32.0f / 256.0f, 1 });
	healthBar.setOutlineColor({ .6,.6,.6,1 }); 
	healthBar.setFractionLineColor({ 114.0f / 256.0f,11.0f / 256.0f,22.0f / 256.0f,1 });
	healthBar.textAlignment = 1; // center;
	healthBar.text.shadow = true;

	positionTextX.text = "X";
	positionTextX.alignX(gui::ALIGN_LEFT);
	positionTextX.size = 1;
	positionTextX.shadow = true;
	positionTextX.color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); //X is blue

	positionTextY.text = "Y";
	positionTextY.alignX(gui::ALIGN_LEFT);
	positionTextY.size = 1;
	positionTextY.shadow = true;
	positionTextY.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); //Y is white

	positionTextZ.text = "Z";
	positionTextZ.alignX(gui::ALIGN_LEFT);
	positionTextZ.size = 1;
	positionTextZ.shadow = true;
	positionTextZ.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); //Z is red

	positionTextW.text = "W";
	positionTextW.alignX(gui::ALIGN_LEFT);
	positionTextW.size = 1;
	positionTextW.shadow = true;
	positionTextW.color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); //W is green

	coordsContainer.xSpacing = 10;
	coordsContainer.xMargin = 0;
	coordsContainer.yMargin = 0;
	coordsContainer.addElement(&positionTextX);
	coordsContainer.addElement(&positionTextY);
	coordsContainer.addElement(&positionTextZ);
	coordsContainer.addElement(&positionTextW);

	textContainer.ySpacing = 12;
	textContainer.yMargin = 6;
	textContainer.elementYOffset = 4;
	textContainer.addElement(&nameText);
	textContainer.addElement(&healthBar);
	tipContainer.addElement(&icon);
	tipContainer.addElement(&textContainer);
	tipContainer.elementYAlign = gui::ALIGN_CENTER_Y;
	tipContainer.alignX(alignmentX);
	tipContainer.alignY(alignmentY);
	tipContainer.xPadding = 5;
	tipContainer.yPadding = 5;
	tipContainer.yMargin = 1;
	tipContainer.xMargin = 5;
	tipContainer.renderBackground = true;

	ui.addElement(&tipContainer);
}


//Render custom UI
$hook(void, Player, renderHud, GLFWwindow* window)
{
	original(self, window);
	if (!isTargeting) return;
	if (self->inventoryManager.secondary) return;

	// technically in the game glDepthMask() is used, but for some reason that doesn't work in mods, thus using glDisable/glEnable instead.
	glDisable(GL_DEPTH_TEST);

	nameText.setText(targetName);

	if (currentTargetMaxHealth>=0) {
		healthBar.setMaxFill(currentTargetMaxHealth);
		healthBar.setFill(currentTargetHealth);
	}

	if (self->isHoldingCompass() && !isDisplayingCoords) {
		isDisplayingCoords = true;
		textContainer.addElement(&coordsContainer);
	}
	else if (!self->isHoldingCompass() && isDisplayingCoords) {
		isDisplayingCoords = false;
		textContainer.removeElement(&coordsContainer);
	}
	else if (self->isHoldingCompass() && !isTargetingBlock) {
		positionTextX.text = std::format("X:{:.1f}", targetPos.x);
		positionTextY.text = std::format("Y:{:.1f}", targetPos.y);
		positionTextZ.text = std::format("Z:{:.1f}", targetPos.z);
		positionTextW.text = std::format("W:{:.1f}", targetPos.w);
	}
	else if (self->isHoldingCompass() && isTargetingBlock) {
		positionTextX.text = std::format("X:{}", targetPos.x);
		positionTextY.text = std::format("Y:{}", targetPos.y);
		positionTextZ.text = std::format("Z:{}", targetPos.z);
		positionTextW.text = std::format("W:{}", targetPos.w);
	}
	icon.tr->setClip(36 * (targetBlockId - 1) + 5, 42, 25, 26); // Not exactly the best place for it but it doesnt work elsewhere
	ui.render();
	glEnable(GL_DEPTH_TEST);
}

void getHealthInfo(Entity* entity, float& currentHealth, float& maxHealth) { //Ridiculous 
	currentHealth = -1;
	maxHealth = -1;
	if (entity->getName()=="Spider") {
		currentHealth = ((EntitySpider*)entity)->health;
		maxHealth = 20;
	}
	else if (entity->getName() == "Butterfly") {
		currentHealth = ((EntityButterfly*)entity)->health;
		maxHealth = 30;
	}
	else if (entity->getName() == "Player") {
		currentHealth = ((EntityPlayer*)entity)->ownedPlayer->health;
		maxHealth = Player::maxHealth;
	}
}

//Update target
$hook(void, Player, updateTargetBlock, World* world, float maxDist)
{
	original(self, world, maxDist);

	Entity* intersectedEntity = world->getEntityIntersection(self->cameraPos, self->reachEndpoint, self->EntityPlayerID);

	if (intersectedEntity != nullptr)
	{
		targetName = intersectedEntity->getName();
		targetPos = intersectedEntity->getPos();
		if (tipContainer.elements[0] == &icon)
			tipContainer.removeElement(&icon);
		isTargeting = true;
		isTargetingBlock = false;
		getHealthInfo(intersectedEntity, currentTargetHealth, currentTargetMaxHealth);
		if (currentTargetMaxHealth < 0 && textContainer.hasElement(&healthBar)) {
			textContainer.removeElement(&healthBar);
		}
		if (currentTargetMaxHealth >= 0 && !textContainer.hasElement(&healthBar)) {
			textContainer.addElement(&healthBar,1);
		}
	}
	else if (self->targetingBlock) {
		targetBlockId = world->getBlock(self->targetBlock);
		targetName = getBlockName(targetBlockId);
		targetPos = self->targetBlock;
		if (tipContainer.elements[0] != &icon)
			tipContainer.addElement(&icon, 0);
		isTargeting = true;
		isTargetingBlock = true;
		isDisplayingHealth = false;
		if (textContainer.hasElement(&healthBar) ){
			textContainer.removeElement(&healthBar);
		}
		currentTargetHealth = -1;
		currentTargetMaxHealth = -1;
	}
	else
		isTargeting = false;
}

void updateConfig(const std::string& path, const nlohmann::json& j)
{
	std::ofstream configFileO(path);
	if (configFileO.is_open())
	{
		configFileO << j.dump(4);
		configFileO.close();
	}
}

//Initialize stuff when launching the game
$hook(void, StateIntro, init, StateManager& s)
{
	original(self, s);

	// initialize opengl stuff
	glewExperimental = true;
	glewInit();
	glfwInit();

	// Thank god 4dkeybinds had config reading code for me to steal

	configPath = std::format("{}/config.json", fdm::getModPath(fdm::modID));

	nlohmann::json configJson
	{
		{ "AlignmentX", alignmentX },
		{ "AlignmentY", alignmentY }
	};

	if (!std::filesystem::exists(configPath))
	{
		updateConfig(configPath, configJson);
	}
	else
	{
		std::ifstream configFileI(configPath);
		if (configFileI.is_open())
		{
			configJson = nlohmann::json::parse(configFileI);
			configFileI.close();
		}
	}

	if (!configJson.contains("AlignmentX"))
	{
		configJson["AlignmentX"] = alignmentX;
		updateConfig(configPath, configJson);
	}
	if (!configJson.contains("AlignmentY"))
	{
		configJson["AlignmentY"] = alignmentY;
		updateConfig(configPath, configJson);
	}

	alignmentX = configJson["AlignmentX"];
	alignmentY = configJson["AlignmentY"];
}

//Hijacking settings or smth
$hook(void, StateSettings, init, StateManager& s)
{
	initializedSettings = false;
	original(self, s);
}

inline static int getY(gui::Element* element)
{
	// Tr1ngle is comparing typeid name strings instead of using dynamic_cast because typeids of 4dminer and typeids of 4dm.h are different
	if (0 == strcmp(typeid(*element).name(), "class gui::Button"))
		return ((gui::Button*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class gui::CheckBox"))
		return ((gui::CheckBox*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class gui::Image"))
		return ((gui::Image*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class gui::Slider"))
		return ((gui::Slider*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class gui::Text"))
		return ((gui::Text*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class gui::TextInput"))
		return ((gui::TextInput*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::Button"))
		return ((gui::Button*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::CheckBox"))
		return ((gui::CheckBox*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::Image"))
		return ((gui::Image*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::Slider"))
		return ((gui::Slider*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::Text"))
		return ((gui::Text*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::TextInput"))
		return ((gui::TextInput*)element)->yOffset;
	return 0;
}

void updateAlignment() {
	switch (alignmentX) {
	case gui::ALIGN_LEFT:
		xSlider.setText("Horizontal Alignment: Left");
		break;
	case gui::ALIGN_RIGHT:
		xSlider.setText("Horizontal Alignment: Right");
		break;
	case gui::ALIGN_CENTER_X:
		xSlider.setText("Horizontal Alignment: Center");
		break;
	}
	switch (alignmentY) {
	case gui::ALIGN_TOP:
		ySlider.setText("Vertical Alignment: Top");
		break;
	case gui::ALIGN_BOTTOM:
		ySlider.setText("Vertical Alignment: Bottom");
		break;
	case gui::ALIGN_CENTER_Y:
		ySlider.setText("Vertical Alignment: Center");
		break;
	}
	tipContainer.alignX(alignmentX);
	tipContainer.alignY(alignmentY);
}

//Mapping slider values to alignment enum values and vice versa
gui::AlignmentX getXAlignment(int value) {
	switch (value) {
	case 0:
		return gui::ALIGN_LEFT;
	case 1:
		return gui::ALIGN_CENTER_X;
	case 2:
		return gui::ALIGN_RIGHT;
	}
	return gui::ALIGN_LEFT;
}
int getXValue(gui::AlignmentX alignment) {
	switch (alignment) {
	case gui::ALIGN_LEFT:
		return 0;
	case gui::ALIGN_CENTER_X:
		return 1;
	case gui::ALIGN_RIGHT:
		return 2;
	}
	return 0;
}
gui::AlignmentY getYAlignment(int value) {
	switch (value) {
	case 0:
		return gui::ALIGN_BOTTOM;
	case 1:
		return gui::ALIGN_CENTER_Y;
	case 2:
		return gui::ALIGN_TOP;
	}
	return gui::ALIGN_BOTTOM;
}
int getYValue(gui::AlignmentY alignment) {
	switch (alignment) {
	case gui::ALIGN_BOTTOM:
		return 0;
	case gui::ALIGN_CENTER_Y:
		return 1;
	case gui::ALIGN_TOP:
		return 2;
	}
	return 0;
}

void initSettings() {
	title.setText("WTHIT Options:");
	title.alignX(gui::ALIGN_CENTER_X);


	xSlider.alignX(gui::ALIGN_CENTER_X);
	
	xSlider.range = 2; // 0,1,2
	xSlider.value = getXValue(alignmentX);
	xSlider.width = 500;
	xSlider.user = &xSlider;
	xSlider.callback = [](void* user, int value)
		{
			alignmentX = getXAlignment(value);
			updateAlignment();
			updateConfig(configPath, { { "AlignmentX", alignmentX },{ "AlignmentY", alignmentY } });
		};



	ySlider.value = getYValue(alignmentY);
	ySlider.alignX(gui::ALIGN_CENTER_X);
	ySlider.range = 2; // 0,1,2
	ySlider.width = 500;
	ySlider.user = &ySlider;
	ySlider.callback = [](void* user, int value)
		{
			alignmentY = getYAlignment(value);
			updateAlignment();
			updateConfig(configPath, { { "AlignmentX", alignmentX },{ "AlignmentY", alignmentY } });
		};

	updateAlignment();
}

void initSettingsWithoutBetterUI(StateSettings *self) {
	int lowestY = 0;
	for (auto& e : self->mainContentBox.elements)
	{
		//wth is a secret button? gonna check it out rn
		if (e == &self->secretButton) // skip the secret button
			continue;

		lowestY = std::max(getY(e), lowestY);
	}
	int oldLowest = lowestY;

	initSettings();

	title.size = 2;
	title.offsetY(lowestY += 100);
	xSlider.offsetY(lowestY += 100);
	ySlider.offsetY(lowestY += 100);

	self->mainContentBox.addElement(&title);
	self->mainContentBox.addElement(&xSlider);
	self->mainContentBox.addElement(&ySlider);

	self->secretButton.yOffset += lowestY - oldLowest;
	self->mainContentBox.scrollH += lowestY - oldLowest;
}

void initSettingsWithBetterUI(StateSettings *self) {
	settingsContainer.clear();

	initSettings();

	title.size = 3;
	title.shadow = true;

	settingsContainer.addElement(&title);
	settingsContainer.addElement(&xSlider);
	settingsContainer.addElement(&ySlider);

	settingsContainer.ySpacing = 20;
	
	BetterUI::getCategoryContainer()->addElement(&settingsContainer, BetterUI::getCategoryContainer()->elements.size()-1);
}

//Add custom settings 
$hook(void, StateSettings, render, StateManager& s)
{
	original(self, s);

	if (initializedSettings)
		return;

	if (fdm::isModLoaded("zihed.betterui")) {
		initSettingsWithBetterUI(self);
	}
	else {
		initSettingsWithoutBetterUI(self);
	}

	initializedSettings = true;

}

// Make auilib work lol
$hook(void, StateGame, windowResize, StateManager& s, GLsizei width, GLsizei height) {
	original(self, s, width, height);
	viewportCallback(s.window, { 0,0, width, height },{0,0});
}

$hook(bool, Player, isHoldingCompass) {
	return original(self) || (self->equipment.getSlot(0).get()!=nullptr && self->equipment.getSlot(0).get()->getName() == "Compass");
}
