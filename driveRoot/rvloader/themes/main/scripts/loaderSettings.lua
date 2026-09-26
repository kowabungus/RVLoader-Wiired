require 'scripts/enum'
require 'scripts/class'
require 'scripts/menuSystem'
require 'scripts/settingsMenu'


loaderSettings = class(SettingsMenu)

function loaderSettings:init(font, lineHeight, columnWidth, sideMargin)
    SettingsMenu.init(self, font, lineHeight, columnWidth, sideMargin)

    self.selectionEmu = enum({"selTheme", "selBackground", "selLoad", "selHome", "saveConfig", "bootPriiloader", "runInstaller"})
    self.selected = self.selectionEmu[1]

    self.availThemes = Theme.getThemes()
    self.curTheme = Theme.getLoadedTheme()
    self.curThemeId = 1

    self.availBackgrounds = Theme.getBackgrounds()
    self.curBackground = Theme.getLoadedBackground()
    self.curBackgroundId = 1

    self.wiiLoadScreen = Theme.getWiiLoadingScreen()
    self.curWiiLoadScreen = self.wiiLoadScreen

    --Home icon target on the C-stick wheel: 0 = Wii Menu, 1 = Wiired
    self.homeTargetNames = {[0] = "Wii Menu", [1] = "Wiired Menu"}
    --Needs a boot.dol built with Sys.getHomeTarget/setHomeTarget; if the running
    --boot.dol is older, show a notice instead of crashing the settings screen
    self.homeTargetAvailable = (Sys.getHomeTarget ~= nil and Sys.setHomeTarget ~= nil)
    if self.homeTargetAvailable then
        self.homeTarget = Sys.getHomeTarget()
    else
        self.homeTarget = 0
    end
    if self.homeTargetNames[self.homeTarget] == nil then
        self.homeTarget = 0
    end
    self.curHomeTarget = self.homeTarget

    for i = 1, #self.availThemes do
        Sys.debug("Theme: " .. self.availThemes[i] .. "\n")
        if self.availThemes[i] == self.curTheme then
            self.curThemeId = i
        end
    end

    if self.curBackground == "None" then
        self.curBackgroundId = 0
    else
        for i = 1, #self.availBackgrounds do
            Sys.debug("Background: " .. self.availBackgrounds[i] .. "\n")
            if self.availBackgrounds[i] == self.curBackground then
                self.curBackgroundId = i
            end
        end
    end

    if #self.availBackgrounds == 0 then
        self.curBackgroundId = 0
    end
end

function loaderSettings:draw(onFocus)
    self.menuSystem:start(onFocus)
    self.menuSystem:printLine("Selected theme:", self.selected.id)
    self.menuSystem:printLineValue(self.availThemes[self.curThemeId], self.availThemes[self.curThemeId] ~= self.curTheme)
    self.menuSystem:printLine("Background image:", self.selected.id)
    if self.curBackgroundId == 0 then
        self.menuSystem:printLineValue("None", "None" ~= self.curBackground)
    else
        self.menuSystem:printLineValue(self.availBackgrounds[self.curBackgroundId], self.availBackgrounds[self.curBackgroundId] ~= self.curBackground)
    end
    self.menuSystem:printLine("Wii game loading screen", self.selected.id)
    if self.curWiiLoadScreen == 0 then
        self.menuSystem:printLineValue("Default (verbose)", self.curWiiLoadScreen ~= self.wiiLoadScreen)
    elseif self.curWiiLoadScreen == 1 then
        self.menuSystem:printLineValue("Spinning Disc", self.curWiiLoadScreen ~= self.wiiLoadScreen)
    end
    self.menuSystem:printLine("Home menu", self.selected.id)
    if self.homeTargetAvailable then
        self.menuSystem:printLineValue(self.homeTargetNames[self.curHomeTarget], self.curHomeTarget ~= self.homeTarget)
    else
        self.menuSystem:printLineValue("Needs updated boot.dol", false)
    end
    self.menuSystem:printLine("Save config", self.selected.id)
    self.menuSystem:printLine("Boot priiloader", self.selected.id)
    self.menuSystem:printLine("Run installer", self.selected.id)
    self.menuSystem:finish()
end

function loaderSettings:handleInputs(onFocus)
    local down = Pad.gendown(0)
    local held = Pad.genheld(0)
    local curId = self.selected.id

    if down.BUTTON_B then
        handlingLeftColumn = true
        return
    end

    if down.BUTTON_DOWN and curId < self.selectionEmu.size then
        curId = curId + 1
    end

    if down.BUTTON_UP and curId > 1 then
        curId = curId - 1
    end

    if down.BUTTON_A then
        if self.selected == self.selectionEmu.selLoad then
            if self.curWiiLoadScreen == 0 then
                self.curWiiLoadScreen = 1
            elseif self.curWiiLoadScreen == 1 then
                self.curWiiLoadScreen = 0
            end
        elseif self.selected == self.selectionEmu.selHome then
            if self.homeTargetAvailable then
                self.curHomeTarget = 1 - self.curHomeTarget
            end
        elseif self.selected == self.selectionEmu.saveConfig then
            local hasToReboot = false
            if self.availThemes[self.curThemeId] ~= self.curTheme then
                Theme.setTheme(self.availThemes[self.curThemeId])
                hasToReboot = true
            end
            if self.curBackgroundId == 0  and "None" ~= self.curBackground then
                Theme.setBackground("None")
                hasToReboot = true
            elseif self.availBackgrounds[self.curBackgroundId] ~= self.curBackground then
                Theme.setBackground(self.availBackgrounds[self.curBackgroundId])
                hasToReboot = true
            end
            if self.curWiiLoadScreen ~= self.wiiLoadScreen then
                Theme.setWiiLoadingScreen(self.curWiiLoadScreen)
                self.wiiLoadScreen = self.curWiiLoadScreen
            end
            if self.homeTargetAvailable and self.curHomeTarget ~= self.homeTarget then
                Sys.setHomeTarget(self.curHomeTarget)
                self.homeTarget = self.curHomeTarget
            end
            if hasToReboot then
                Sys.reboot()
            end
        elseif self.selected == self.selectionEmu.bootPriiloader then
            Sys.bootPriiloader()
        elseif self.selected == self.selectionEmu.runInstaller then
            Sys.bootInstaller()
        end
    elseif down.BUTTON_RIGHT then
        if self.selected == self.selectionEmu.selTheme then
            self.curThemeId = self.curThemeId + 1
            if self.curThemeId > #self.availThemes then
                self.curThemeId = 1
            end
        elseif self.selected == self.selectionEmu.selBackground then
            self.curBackgroundId = self.curBackgroundId + 1
            if self.curBackgroundId > #self.availBackgrounds then
                self.curBackgroundId = 0
            end
        elseif self.selected == self.selectionEmu.selLoad then
            if self.curWiiLoadScreen == 0 then
                self.curWiiLoadScreen = 1
            elseif self.curWiiLoadScreen == 1 then
                self.curWiiLoadScreen = 0
            end
        elseif self.selected == self.selectionEmu.selHome then
            if self.homeTargetAvailable then
                self.curHomeTarget = 1 - self.curHomeTarget
            end
        end
    elseif down.BUTTON_LEFT then
        if self.selected == self.selectionEmu.selTheme then
            self.curThemeId = self.curThemeId - 1
            if self.curThemeId < 1 then
                self.curThemeId = #self.availThemes
            end
        elseif self.selected == self.selectionEmu.selBackground then
            self.curBackgroundId = self.curBackgroundId - 1
            if self.curBackgroundId < 0 then
                self.curBackgroundId = #self.availBackgrounds
            end
        elseif self.selected == self.selectionEmu.selLoad then
            if self.curWiiLoadScreen == 0 then
                self.curWiiLoadScreen = 1
            elseif self.curWiiLoadScreen == 1 then
                self.curWiiLoadScreen = 0
            end
        elseif self.selected == self.selectionEmu.selHome then
            if self.homeTargetAvailable then
                self.curHomeTarget = 1 - self.curHomeTarget
            end
        end
    end

    self.selected = self.selectionEmu[curId]
end
