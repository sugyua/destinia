#pragma once
#include <algorithm>
#include <cstdint>
#include <cctype>
#include <functional>
#include <map>
#include <string>
#include <vector>

// Native font assets currently require ASCII. Do not insert UTF-8 into these tables.
namespace destinia {
inline const std::map<std::string, std::string>& portugueseText() {
  static const std::map<std::string, std::string> translations = {
      {"Abyssal Hollow", "Abismo Profundo"},
      {"Ancient Ruins", "Ruinas Antigas"},
      {"Attila Highlands", "Terras Altas de Attila"},
      {"Deserted Cave", "Caverna Abandonada"},
      {"Dungeon Prison", "Prisao Subterranea"},
      {"East Abyssal Forest", "Floresta Abissal Leste"},
      {"East Croesus Woods", "Bosque Croesus Leste"},
      {"East Parma Plateau", "Planalto Parma Leste"},
      {"Grudge Cavern", "Caverna do Rancor"},
      {"Lagrant Castle Dungeon", "Masmorra do Castelo Lagrant"},
      {"Mushroom Cave", "Caverna dos Cogumelos"},
      {"Mystic Crystal Cave", "Caverna do Cristal Mistico"},
      {"North Arubis Mountains", "Montanhas Arubis Norte"},
      {"North Berwin Woodlands", "Bosque Berwin Norte"},
      {"North Darkwoods", "Bosque Sombrio Norte"},
      {"North Feralas Mountains", "Montanhas Feralas Norte"},
      {"North Maze Forest", "Floresta Labirinto Norte"},
      {"North Savanna", "Savana Norte"},
      {"Salas Lair", "Covil de Salas"},
      {"Shadow Cavern", "Caverna das Sombras"},
      {"South Arubis Mountains", "Montanhas Arubis Sul"},
      {"South Berwin Woodlands", "Bosque Berwin Sul"},
      {"South Darkwoods", "Bosque Sombrio Sul"},
      {"South Feralas Mountains", "Montanhas Feralas Sul"},
      {"South Maze Forest", "Floresta Labirinto Sul"},
      {"South Savanna", "Savana Sul"},
      {"Spirit Den", "Covil dos Espiritos"},
      {"West Abyssal Forest", "Floresta Abissal Oeste"},
      {"West Croesus Woods", "Bosque Croesus Oeste"},
      {"West Feralas Mountains", "Montanhas Feralas Oeste"},
      {"West Parma Plateau", "Planalto Parma Oeste"},
      {"Abyssal Hollow Entrance", "Entrada: Abismo Profundo"},
      {"Ancient Ruins Entrance", "Entrada: Ruinas Antigas"},
      {"Deserted Cave Entrance", "Entrada: Caverna Abandonada"},
      {"Grudge Cavern Entrance", "Entrada: Caverna do Rancor"},
      {"Mushroom Cave Entrance", "Entrada: Caverna dos Cogumelos"},
      {"Mystic Crystal Cave Entrance", "Entrada: Caverna do Cristal Mistico"},
      {"Salas Lair Entrance", "Entrada: Covil de Salas"},
      {"Shadow Cavern Entrance", "Entrada: Caverna das Sombras"},
      {"Spirit Den Entrance", "Entrada: Covil dos Espiritos"},
      {"Basic Controls", "Controles basicos"},
      {"Move Character: Direction key icons\nSelect Menu: OK icon\nPrevious Menu: Cancel icon\nWorld Map: Mini-map icon\nSkip Dialogue: Skip icon", "Mover personagem: icones direcionais\\nSelecionar menu: icone OK\\nMenu anterior: icone Cancelar\\nMapa-mundi: icone do minimapa\\nPular dialogo: icone Pular"},
      {"Character Stats", "Atributos do personagem"},
      {"STR: Increases Physical Attack Strength and Defense.\nDEX: Increases Long-ranged Attack Strength, Critical Attack Chance, and Evasion Rate.\nVIT: Increases HP.\nINT: Increases MP.\n1 stat point is awarded each time you increase your level. Use stat points to improve your abilities.", "FOR: aumenta o ataque fisico e a defesa.\\nDES: aumenta o ataque a distancia, a chance de critico e a esquiva.\\nVIT: aumenta os PV.\\nINT: aumenta os PM.\\nCada nivel concede 1 ponto de atributo para melhorar suas capacidades."},
      {"Skills are automatically learned as you increase your level.\nYou can perform skills by using hotkeys. Skills consume MP each time they are used.\nUse the Specialty menu to improve your skills.", "Voce aprende habilidades ao subir de nivel.\\nUse os atalhos para ativar habilidades. Cada uso consome PM.\\nMelhore suas habilidades no menu Especialidade."},
      {"Equipment items are classified into 5 grades. You can check your item's grade by the color of its name.\nNormal: White\nMagic: Green\nRare: Blue\nEpic: Purple\nLegendary: Orange", "Equipamentos possuem 5 graus. A cor do nome indica o grau.\\nNormal: branco\\nMagico: verde\\nRaro: azul\\nEpico: roxo\\nLendario: laranja"},
      {"Reinforcement/Composition", "Reforco e composicao"},
      {"You can attempt to reinforce equipment items by using Forging Stones.\nIf your attempt to reinforce an item fails, then the item may lower in grade or may even be destroyed.\nCreate new equipment by compounding equipment items of the same grade.\nIf you get lucky, you might receive a higher-graded item.", "Use Pedras de Forja para reforcar equipamentos.\\nSe o reforco falhar, o grau pode diminuir e o equipamento pode ser destruido.\\nCombine equipamentos do mesmo grau para criar outros.\\nCom sorte, voce pode obter um grau superior."},
      {"Gem Equipment", "Gemas no equipamento"},
      {"You can also enhance the abilities of your equipment by adding gems to equipment sockets.\nYou can, however, fail when equipping a gem. If you fail to equip a gem, then all existing gems in the equipment item are removed.", "Adicione gemas aos encaixes para melhorar o equipamento.\\nA colocacao de uma gema pode falhar. Nesse caso, todas as gemas ja presentes no equipamento sao removidas."},
      {"Craft", "Fabricacao"},
      {"Use Recipes to create items and equipment.\nYou can obtain craft materials by hunting monsters or disassembling equipment.", "Use receitas para criar itens e equipamentos.\\nObtenha materiais ao derrotar monstros ou desmontar equipamentos."},
      {"Hatch a monster egg to acquire a pet.Each pet has its own set of unique skills to aid you in battle.\nThe levels of pets increase as they evolve.", "Choque um ovo de monstro para obter um mascote. Cada mascote possui habilidades para ajudar no combate.\\nO nivel do mascote aumenta quando ele evolui."},
      {"There are two types of quests: the main quests are mandatory while sub-quests are optional.All quests offer a generous amount of EXP and rewards, so we recommend that you complete all the quests that you can.", "As missoes principais sao obrigatorias; as secundarias sao opcionais. Todas concedem EXP e recompensas. Tente concluir todas as que puder."},
      {"PvP", "PvP"},
      {"You can duel with other players through Multi Mode > Battle Arena.\nThe winners are awarded Honor Points.You can use Honor Points to purchase special items at the Honor Shop.", "Duele com outros jogadores em Modo Multi > Arena de Batalha.\\nVitorias concedem Pontos de Honra para comprar itens especiais na Loja de Honra."},
      {"In the Multi Mode, you can duel with other players, trade or gift items, and check your ranking.", "No Modo Multi, voce pode duelar, trocar ou presentear itens e consultar sua classificacao."},
      {"Fungus", "Fungo"}, {"Magic Fungus", "Fungo Magico"}, {"Dark Fungus", "Fungo Sombrio"},
      {"Scorpion", "Escorpiao"}, {"King Scorpion", "Rei Escorpiao"}, {"Demon Scorpion", "Escorpiao Demonio"},
      {"Curimon", "Curimon"}, {"Magic Curimon", "Curimon Magico"}, {"Dark Curimon", "Curimon Sombrio"},
      {"Piccolo", "Piccolo"}, {"Magic Piccolo", "Piccolo Magico"}, {"Dark Piccolo", "Piccolo Sombrio"},
      {"Mimic", "Mimico"}, {"Dark Mimic", "Mimico Sombrio"}, {"Chaos Mimic", "Mimico do Caos"},
      {"Fanatic", "Fanatico"}, {"Shadow Follower", "Seguidor Sombrio"},
      {"Dark Follower", "Seguidor Sombrio"}, {"Tentacle Mon", "Monstro Tentaculo"},
      {"King Tentacle", "Rei Tentaculo"}, {"Shadow Tentacle", "Tentaculo Sombrio"},
      {"Sasquatch", "Sasquatch"}, {"King Sasquatch", "Rei Sasquatch"},
      {"Demon Sasquatch", "Sasquatch Demonio"}, {"Black Mage", "Mago Negro"},
      {"Shadow Sorcerer", "Feiticeiro Sombrio"}, {"Dark Mage", "Mago Sombrio"},
      {"Skeleton", "Esqueleto"}, {"Magic Skeleton", "Esqueleto Magico"},
      {"Dark Skeleton", "Esqueleto Sombrio"}, {"Carnivore Wasp", "Vespa Carnivora"},
      {"Venomous Honey Bee", "Abelha Venenosa"}, {"King Wasp", "Vespa Rainha"},
      {"Ramu", "Ramu"}, {"Magic Ramu", "Ramu Magico"}, {"Dark Ramu", "Ramu Sombrio"},
      {"Magic Slime", "Gosma Magica"}, {"Demon Slime", "Gosma Demonio"},
      {"Dark Slime", "Gosma Sombria"}, {"Fire Spirit", "Espirito de Fogo"},
      {"Flame Spirit", "Espirito Flamejante"}, {"Lava Spirit", "Espirito de Lava"},
      {"Lycanthrope", "Licantropo"}, {"Dark Lycanthrope", "Licantropo Sombrio"},
      {"Demon Lycanthrope", "Licantropo Demonio"}, {"Bearman", "Homem-Urso"},
      {"Magic Bearman", "Homem-Urso Magico"}, {"Dark Bearman", "Homem-Urso Sombrio"},
      {"Magic Worm", "Verme Magico"}, {"Demon Worm", "Verme Demonio"},
      {"Great Worm", "Verme Gigante"}, {"Pumpkin Soldier", "Soldado Abobora"},
      {"Pumpkin Ghost", "Fantasma Abobora"}, {"Dark Pumpkin", "Abobora Sombria"},
      {"Frost Spirit", "Espirito de Geada"}, {"Ice Spirit", "Espirito de Gelo"},
      {"Ice Spirit ", "Espirito de Gelo "}, {"Glacier Spirit", "Espirito Glacial"},
      {"Zephyr", "Zefiro"}, {"Magic Zephyr", "Zefiro Magico"}, {"Dark Zephyr", "Zefiro Sombrio"},
      {"Imperial Soldier", "Soldado Imperial"}, {"Deserter Soldier", "Soldado Desertor"},
      {"Elite Soldier", "Soldado de Elite"}, {"Dungeon Guardian", "Guardiao da Caverna"},
      {"Defcan", "Defcan"}, {"Salas", "Salas"}, {"Sileth", "Sileth"},
      {"Berserk Tamur", "Tamur Furioso"}, {"Kaiser", "Kaiser"}, {"Baramos", "Baramos"},
      {"Diablo", "Diablo"}, {"Lava Guardian", "Guardiao da Lava"},
      {"Inventory", "Inventario"}, {"Equipment", "Equipamento"}, {"Equip", "Equipar"},
      {"Unequip", "Desequipar"}, {"Items", "Itens"}, {"Item", "Item"},
      {"Skills", "Habilidades"}, {"Skill", "Habilidade"}, {"Making", "Fabricacao"},
      {"Quest", "Missao"}, {"Quests", "Missoes"}, {"Main Menu", "Menu Principal"},
      {"Pause", "Pausa"}, {"Resume", "Continuar"}, {"Option", "Opcoes"},
      {"Options", "Opcoes"}, {"Save", "Salvar"}, {"Load", "Carregar"},
      {"Back", "Voltar"}, {"Exit", "Sair"}, {"Confirm", "Confirmar"},
      {"Cancel", "Cancelar"}, {"Select", "Selecionar"}, {"Use", "Usar"},
      {"Sell", "Vender"}, {"Buy", "Comprar"}, {"Create", "Criar"},
      {"Combine", "Combinar"}, {"Enhance", "Aprimorar"}, {"Appraise", "Identificar"},
      {"Pet", "Mascote"}, {"Status", "Status"}, {"Party", "Grupo"},
      {"BACK", "VOLTAR"}, {"Quest completed:<C0>%s<C>", "Missao concluida:<C0>%s<C>"},
      {"Want to accept this quest?", "Deseja aceitar esta missao?"},
      {"The quest accepted.", "Missao aceita."}, {"This quest completed.", "Missao concluida."},
      {"A new skill acquired.", "Nova habilidade adquirida."},
      {"You cannot complete this quest because your Inventory is full.", "Nao e possivel concluir esta missao: inventario cheio."},
      {"Your Inventory is full.", "Seu inventario esta cheio."},
      {"Has high Attack Strength and Vitality.\nMain Weapon: Giant Sword", "Possui alta Forca de Ataque e Vitalidade.\nArma Principal: Espada Grande"},
      {"Has high Dexterity and specializes in long-ranged attacks.\nMain Weapon: Gun", "Possui alta Destreza e e especialista em ataques a distancia.\nArma Principal: Arma de Fogo"},
      {"Has fast Attack and Movement Speeds.\nMain Weapon: One-handed Sword", "Possui alta velocidade de Ataque e Movimento.\nArma Principal: Espada de Uma Mao"},
      {"Equipment", "Equipamento"}, {"Select an item to compound.", "Selecione um item para combinar."},
      {"Select an item to fit an orb.", "Selecione um item para encaixar uma orbe."},
      {"Select an item to reinforce.", "Selecione um item para reforcar."},
      {"Select an item to identify. ", "Selecione um item para identificar. "},
      {"Select an item to reset.", "Selecione um item para redefinir."},
      {"Select an item to remove orbs.", "Selecione um item para remover orbes."},
      {"Active Skill: %s[LV%d]", "Habilidade Ativa: %s[Nv%d]"},
      {"Passive Skill: %s[LV%d]", "Habilidade Passiva: %s[Nv%d]"},
      {"%s Making", "Fabricacao de %s"}, {"Use Recipes to create equipment and items.", "Use receitas para criar equipamentos e itens."},
      {"Weapon", "Arma"}, {"Greatsword", "Espada Grande"}, {"Great Sword", "Espada Grande"},
      {"Sword", "Espada"}, {"Armor", "Armadura"},
      {"[Materials]", "[Materiais]"}, {"Material", "Material"},
      {"Select an advanced class.", "Selecione uma classe avancada."},
      {"Please select a price.", "Selecione um preco."},
      {"Select a price.", "Selecione um preco."},
      {"Compound items. (Auto-save)", "Combine itens. (Salvamento automatico)"},
      {"Disassemble an item. (Auto-save)", "Desmonte um item. (Salvamento automatico)"},
      {"Identify an item. (Auto-save)", "Identifique um item. (Salvamento automatico)"},
      {"Equip an orb. (Auto-save)", "Equipe uma orbe. (Salvamento automatico)"},
      {"Reset the status of an identified item.\n(Auto-save)", "Redefina os atributos de um item identificado.\n(Salvamento automatico)"},
      {"Reset the effects of an orb.\n(Auto-save)", "Redefina os efeitos de uma orbe.\n(Salvamento automatico)"},
      {"Attempt to reinforce an item.(Auto-save) Failing to reinforce the item can lower its grade or even destroy it.", "Tente reforcar um item. (Salvamento automatico) Uma falha pode reduzir sua qualidade ou ate destrui-lo."},
      {"Attempt to reinforce an item.(Auto-save) Failing to reinforce the item can lower its grade.", "Tente reforcar um item. (Salvamento automatico) Uma falha pode reduzir sua qualidade."},
      {"Using the item automatically saves the game.", "Usar este item salva o jogo automaticamente."},
      {"Open the box and obtain an item.\n(Auto-save)", "Abra a caixa e obtenha um item.\n(Salvamento automatico)"},
      {"Reset all of your stats.\n(Auto-save)", "Redefina todos os seus atributos.\n(Salvamento automatico)"},
      {"Reset all of your specialties.\n(Auto-save)", "Redefina todas as suas especialidades.\n(Salvamento automatico)"},
      {"Do you want to abandon this quest?", "Deseja abandonar esta missao?"},
      {"Select Character", "Selecionar Personagem"},
      {"BACK", "VOLTAR"}, {"Select a quantity(Auto save)", "Selecione a quantidade (salvamento automatico)"},
      {"Please select a quantity.", "Selecione uma quantidade."},
      {"Please select an item to gift.", "Selecione um item para presentear."},
      {"Please select an item to sell.", "Selecione um item para vender."},
      {"You cannot sell equipped items.", "Nao e possivel vender itens equipados."},
      {"The item's sale has been cancelled.", "A venda do item foi cancelada."},
      {"Please select a title.", "Selecione um titulo."},
      {"Main Menu", "Menu Principal"}, {"Special Shop", "Loja Especial"},
      {"Weapon Making", "Fabricacao de Armas"}, {"Armor Making", "Fabricacao de Armaduras"},
      {"Greatsword Making", "Fabricacao de Espadas Grandes"},
      {"Sword Making", "Fabricacao de Espadas"},
      {"Making of Weapon", "Fabricacao de Armas"},
      {"Making of Armor", "Fabricacao de Armaduras"},
      {"Making of Greatsword", "Fabricacao de Espadas Grandes"},
      {"Manufacturing", "Fabricacao"}, {"Quest List", "Lista de Missoes"},
      {"Quest completed", "Missao concluida"}, {"Quest accepted", "Missao aceita"},
      {"Active Skill", "Habilidade Ativa"}, {"Passive Skill", "Habilidade Passiva"},
      {"Skill Point", "Ponto de Habilidade"}, {"Skill Points", "Pontos de Habilidade"},
      {"Attribute", "Atributo"}, {"Attributes", "Atributos"},
      {"Specialty", "Especialidade"}, {"Specialties", "Especialidades"},
      {"Character", "Personagem"}, {"Character Status", "Status do Personagem"},
      {"Game Menu", "Menu do Jogo"}, {"Continue", "Continuar"},
      {"System", "Sistema"}, {"Help", "Ajuda"}, {"Quest Info", "Info da Missao"},
      {"Making Menu", "Menu de Fabricacao"}, {"Skill Menu", "Menu de Habilidades"},
      {"Item Menu", "Menu de Itens"}, {"Quest Menu", "Menu de Missoes"},
      {"Quest item.", "Item de missao."},
      {"Inferior Appraisal Scroll", "Perg. Ident. Inferior"},
      {"This can be used to identify Inferior equipment (Lv 1 ~ 19) ", "Identifica equipamento inferior (Nv. 1 a 19)"},
      {"Resurrection Scroll", "Pergaminho de Ressurreicao"},
      {"This can be used to instantly resurrect.", "Ressuscita instantaneamente."},
      {"Special Forging Stone", "Pedra de Forja Especial"},
      {"This can be used to reinforce equipment at any level. It safeguards equipment against breakage upon reinforcement failur}", "Reforca equipamentos de qualquer nivel e evita sua quebra em caso de falha."},
      {"Gold", "Ouro"}, {"Cash", "Creditos"}, {"Yes", "Sim"}, {"No", "Nao"},
      {"Return", "Voltar"}, {"Start", "Iniciar"}, {"Setting", "Configuracao"},
      {"Sound", "Som"}, {"Music", "Musica"}, {"On", "Ligado"}, {"Off", "Desligado"},
      {"Lv", "Nv"}, {"HP", "PV"}, {"MP", "PM"},
      {"Warrior", "Guerreiro"}, {"Shooter", "Atirador"}, {"Assassin", "Assassino"},
      {"Main Weapon", "Arma Principal"}, {"One-handed Sword", "Espada de Uma Mao"},
      {"Giant Sword", "Espada Grande"}, {"Long-ranged", "Longo Alcance"},
      {"Dash Attack", "Ataque Rapido"}, {"Regenerative Aura", "Aura Regenerativa"},
      {"Slash", "Corte"}, {"Crushing Strike", "Golpe Esmagador"},
      {"Combative Aura", "Aura de Combate"}, {"Fearful Shout", "Grito Amedrontador"},
      {"Fire Sword", "Espada de Fogo"}, {"Grinding Strike", "Golpe Triturador"},
      {"Immunity Aura", "Aura de Imunidade"}, {"Destructive Shout", "Grito Destrutivo"},
      {"Twister", "Tornado"}, {"Colossal Blow", "Golpe Colossal"},
      {"Meteor Strike", "Ataque Meteoro"}, {"Power Shot", "Tiro Poderoso"},
      {"Wisdom Aura", "Aura da Sabedoria"}, {"Aimed Shot", "Tiro Preciso"},
      {"Stun Gun", "Arma Atordoante"}, {"Wind Aura", "Aura do Vento"},
      {"Weakening Shout", "Grito Enfraquecedor"}, {"Wild Shot", "Tiro Selvagem"},
      {"Stun Wave", "Onda Atordoante"}, {"Ruthless Aura", "Aura Implacavel"},
      {"Foggy Shout", "Grito Nebuloso"}, {"Summon Machine Gun", "Invocar Metralhadora"},
      {"Finishing Shot", "Tiro Final"}, {"Hyper Bazooka", "Bazuca Hiper"},
      {"Asura Slash", "Corte Asura"}, {"Slaying Aura", "Aura Assassina"},
      {"Rupturing Blow", "Golpe Rupturante"}, {"Jump Attack", "Ataque Saltando"},
      {"Evasive Aura", "Aura Evasiva"}, {"Intimidating Shout", "Grito Intimidador"},
      {"Whirling Slash", "Corte Giratorio"}, {"Thunderstorm", "Tempestade"},
      {"Berserk Aura", "Aura Berserk"}, {"Killer Shout", "Grito Mortal"},
      {"Hiding", "Esconderijo"}, {"Frenzied Hacking", "Golpe Frenetico"},
      {"Star Fall", "Queda Estelar"}, {"Meteor", "Meteoro"},
      {"Cleverness", "Astucia"}, {"Bodybuilding", "Condicionamento"},
      {"Quickness", "Agilidade"}, {"Veteran Shooter", "Atirador Veterano"},
      {"Accurate Fire", "Tiro Preciso"}, {"Explosive Power", "Poder Explosivo"},
      {"Evasion Tactic", "Tatica Evasiva"}, {"Combat Strategy", "Estrategia de Combate"},
      {"Deadly Wound", "Ferida Mortal"}, {"Killer Instinct", "Instinto Assassino"},
      {"Destructive Power", "Poder Destrutivo"}, {"Ruthlessness", "Crueldade"},
      {"Regeneration", "Regeneracao"}, {"Intimidate", "Intimidar"},
      {"Crush", "Esmagar"}, {"Absorb", "Absorver"}, {"Training", "Treinamento"},
      {"Destruct", "Destruir"}, {"Solid", "Resistencia"}, {"Swift", "Rapidez"},
      {"Slaughter", "Massacre"}, {"Heal", "Curar"},
      {"Attack Strength", "Forca de Ataque"}, {"Critical Attack Chance", "Chance de Ataque Critico"},
      {"Critical damage", "Dano critico"}, {"Evasion Rate", "Taxa de Esquiva"},
      {"Accuracy Rate", "Precisao"}, {"Movement Speed", "Velocidade de Movimento"},
      {"Damage Receipt", "Dano Recebido"}, {"Damage", "Dano"},
      {"Defense", "Defesa"}, {"Attack", "Ataque"}, {"Bleeding damage", "Dano de sangramento"},
      {"Stun for", "Atordoa por"}, {"Stun", "Atordoamento"},
      {"Frost", "Geada"}, {"Fang", "Presa"}, {"Blood", "Sangue"},
      {"Rush", "Investida"}, {"Skull", "Craneo"}, {"Penetrator", "Perfurador"},
      {"Soul", "Alma"}, {"Destroyer", "Destruidor"}, {"Mad", "Insano"},
      {"Crusader", "Cruzado"}, {"Deathly", "Mortal"}, {"Guide", "Guia"},
      {"Raging", "Furioso"}, {"Storm", "Tempestade"}, {"Bringer", "Portador"},
      {"Hand", "Mao"}, {"Small", "Pequeno"}, {"Shield", "Escudo"},
      {"Heaven", "Ceu"}, {"Lethal", "Letal"}, {"Demolisher", "Demolidor"},
      {"Anti-frost", "Antigelo"}, {"Fiend", "Demonio"}, {"Hell", "Inferno"},
      {"Infinite", "Infinito"}, {"Vampire", "Vampiro"}, {"Vampiric Rate", "Taxa de Vampirismo"},
      {"Lv ", "Nv. "},
      {"sec.", "s"}, {"seconds", "segundos"}, {"second", "segundo"},
      {"Increase the duration of", "Aumenta a duracao de"},
      {"Reduce the downtime of", "Reduz o tempo de recarga de"},
      {"Reduce the MP consumption of", "Reduz o consumo de PM de"},
      {"Create a %d& chance of Stunning a target when", "Cria %d& de chance de Atordoar o alvo quando"},
      {"Increase the Critical Attack Chance of", "Aumenta a chance de Ataque Critico de"},
      {"Increase your", "Aumenta sua"}, {"Increase your HP Recovery Speed.", "Aumenta a velocidade de recuperacao de PV."},
      {"Increase your MP Recovery Speed.", "Aumenta a velocidade de recuperacao de PM."},
      {"Increase your Evasion Rate.", "Aumenta sua Taxa de Esquiva."},
      {"Increase your Attack Strength.", "Aumenta sua Forca de Ataque."},
      {"Increase your Critical Attack Chance by 5% for 60 seconds.", "Aumenta sua chance de Ataque Critico em 5% por 60 segundos."},
      {"by %d seconds.", "em %d segundos."}, {"by %d second.", "em %d segundo."},
      {"by %d&.", "em %d&."}, {"by %d.", "em %d."},
      {"for %d seconds.", "por %d segundos."},
      {"Damage x", "Dano x"}, {"% Damage", "% de Dano"},
      {"% Bleeding damage for", "% de Dano de sangramento por"},
      {"HP Recovery", "Recuperacao de PV"}, {"MP Recovery", "Recuperacao de PM"},
      {"Instantly restores ", "Restaura instantaneamente "},
      {"Restores ", "Restaura "}, {"Adds ", "Adiciona "},
      {"Provides ", "Concede "}, {"This can be used to create ", "Use para criar "},
      {"This can be used to reinforce ", "Use para reforcar "},
      {"This can be used to identify ", "Use para identificar "},
      {"when equipped to an", "ao equipar em um"},
      {"equipment item", "equipamento"},
      {"any level", "qualquer nivel"},
      {"Inventory slots", "espacos no inventario"},
      {"HP over", "PV ao longo de"},
      {"MP over", "PM ao longo de"},
      {" HP.", " PV."}, {" MP.", " PM."},
      {" HP ", " PV "}, {" MP ", " PM "},
      {"HP and MP", "PV e PM"},
      {"HP Recovery", "recuperacao de PV"},
      {"MP Recovery", "recuperacao de PM"},
      {"Critical Attack Chance", "chance de Ataque Critico"},
      {"Accuracy Rate", "Precisao"},
      {"Evasion Rate", "Taxa de Esquiva"},
      {"Vampiric Rate", "Taxa de Vampirismo"},
      {"Strength", "Forca"}, {"Dexterity", "Destreza"},
      {"Vitality", "Vitalidade"}, {"Intelligence", "Inteligencia"},
      {"Damage", "Dano"}, {"Defense", "Defesa"},
      {"Restores your", "Recupera seu"},
      {"Resurrect", "Ressuscitar"},
      {"Breakage", "Quebra"},
      {"reinforcement failure", "falha no reforco"},
      {"upon reinforcement failure", "em caso de falha no reforco"},
      {"create equipment", "fabricar equipamentos"},
      {"create potions", "fabricar pocoes"},
      {"create corresponding item", "criar o item correspondente"},
      {"reset its status to unidentified", "remover sua identificacao"},
      {"removes all orbs", "remove todas as orbes"},
      {"safeguards equipment against breakage", "evita que o equipamento quebre"},
      {"produces a", "gera uma"},
      {"produces an", "gera um"},
      {"latest village", "ultima vila"},
      {"This can be used on identified equipment at all levels to reset its status to unidentified.", "Use para remover a identificacao de equipamentos de qualquer nivel."},
      {"Use this recipe to create the corresponding item.", "Use esta receita para criar o item correspondente."},
      {"Produces ", "Gera "}, {"Returns to your latest village.", "Retorna a sua ultima vila."},
      {"Removes all orbs from an equipment item.", "Remove todas as orbes de um equipamento."},
      {"Resets your stats.", "Redefine seus atributos."},
      {"Resets your specialties.", "Redefine suas especialidades."},
      {"Instantly resurrect.", "Ressuscita instantaneamente."},
      {" over ", " ao longo de "}, {" at any level.", " de qualquer nivel."},
      {" at all levels ", " de qualquer nivel "}, {" (Lv ", " (Nv. "},
      {" ~ ", " a "}, {"seconds.", "segundos."}, {" seconds.", " segundos."},
      {"% Damage Receipt", "% de Dano Recebido"}, {"Critical damage", "dano Critico"},
      {"Critical Attack", "Ataque Critico"}, {"inflicts", "causa"},
      {"weakest point", "ponto mais fraco"}, {"area-effective damage", "dano em area"},
      {"friendly forces", "aliados"}, {"friendly forcey forces", "aliados"},
      {"all skills", "todas as habilidades"}, {"item drop rate", "chance de obter itens"},
      {"gold drop rate", "chance de obter ouro"}, {"Attack with", "Ataque com"},
      {"decreasing an enemy's", "reduzir a"}, {"Stunning", "Atordoar"},
      {"Stuns", "Atordoa"}, {"chance of", "chance de"}, {"when ", "quando "},
      {"while ", "enquanto "}, {"target", "alvo"}, {"bullets", "balas"},
      {"bullet", "bala"}, {"magic", "magia"}, {"powerful", "poderoso"},
      {"fire ", "disparar "}, {"Fire ", "Fogo "}, {"enemy's", "do inimigo"},
      {"your ", "seu "}, {"his ", "seu "}, {" to ", " para "},
      {" with ", " com "}, {" and ", " e "}, {"Attack Strength x", "Forca de Ataque x"},
      {"Dash forward to perform a triple attack.", "Avance rapidamente para executar um ataque triplo."},
      {"Shoot spirit swords to attack enemies in front of you.", "Dispare espadas espirituais contra os inimigos a sua frente."},
      {"Forcefully strike an enemy to inflict damage and cause Bleeding.", "Acerte um inimigo com forca para causar dano e sangramento."},
      {"Shout to decrease the Movement Speed of surrounding enemies.", "Grite para reduzir a velocidade dos inimigos ao redor."},
      {"Infuse your sword with the power of fire to strike surrounding enemies.", "Imbua sua espada com fogo e ataque os inimigos ao redor."},
      {"Cause Bleeding to surrounding enemies.", "Causa sangramento nos inimigos ao redor."},
      {"Relieve your pain and reduce the amount of damage received from enemies.", "Reduz o dano recebido dos inimigos."},
      {"Shout to decrease the Defense of surrounding enemies.", "Grite para reduzir a Defesa dos inimigos ao redor."},
      {"Inflict multiple damage with a spinning attack.", "Cause varios danos com um ataque giratorio."},
      {"Jump high to deliver a deadly blow.", "Salte alto e desfira um golpe mortal."},
      {"Summon a meteor that inflicts area-effective damage.", "Invoque um meteoro que causa dano em area."},
      {"Infuse your gun with magic to fire powerful bullets.", "Imbua sua arma com magia para disparar balas poderosas."},
      {"Increase your MP Recovery Speed.", "Aumenta a velocidade de recuperacao de PM."},
      {"Attack an enemy's at his weakest point.", "Ataque o inimigo em seu ponto fraco."},
      {"Attack and Stun enemies in front of you.", "Ataque e atordoe os inimigos a sua frente."},
      {"Increase your Evasion Rate.", "Aumenta sua Taxa de Esquiva."},
      {"Weaken surrounding enemies to decrease their Defense.", "Enfraquece os inimigos ao redor e reduz sua Defesa."},
      {"Perform rapid firing attacks at all surrounding enemies.", "Dispare rapidamente contra todos os inimigos ao redor."},
      {"Fire a magic bullet that Stuns surrounding enemies.", "Dispare uma bala magica que atordoa os inimigos ao redor."},
      {"Decrease the enemy's Accuracy Rate by fogging up their vision.", "Reduz a Precisao do inimigo ao obscurecer sua visao."},
      {"Summon a powerful machine gun. ", "Invoque uma metralhadora poderosa. "},
      {"Finish a target with a powerful bullet.", "Derrote o alvo com uma bala poderosa."},
      {"Perform powerful multi-attacks to enemies in front of you.", "Execute varios ataques poderosos contra os inimigos a sua frente."},
      {"Inflict lethal damage on an enemy by stabbing his body.", "Cause dano letal ao perfurar o inimigo."},
      {"Jump-attack and Stun a target.", "Salte, ataque e atordoe o alvo."},
      {"Shout to intimidate an enemy, causing him to lose his will to fight.", "Intimide o inimigo com um grito e reduza sua vontade de lutar."},
      {"Infuse your sword with Fire magic to inflict area-effective damage.", "Imbua sua espada com magia de Fogo para causar dano em area."},
      {"Attack and Stun surrounding enemies.", "Ataque e atordoe os inimigos ao redor."},
      {"Unleash your rage to increase Attack Strength.", "Liberte sua furia para aumentar a Forca de Ataque."},
      {"While hiding from enemy detection, perform Stunning attacks with tremendous damage.", "Fique oculto e execute ataques atordoantes que causam muito dano."},
      {"Hack an enemy to inflict major damage.", "Golpeie o inimigo e cause muito dano."},
      {"Attack with a %d& chance of Stunning an enemy for %d.%d second. This skill lasts for 10 seconds.", "Ataca com %d& de chance de atordoar o inimigo por %d.%d segundo. Dura 10 segundos."},
      {"Attack with a %d& chance of decreasing an enemy's Attack Strength by %d& for 10 seconds. This skill lasts for 10 seconds.", "Ataca com %d& de chance de reduzir a Forca de Ataque do inimigo em %d& por 10 segundos. Dura 10 segundos."},
      {"Attack with a %d& chance of decreasing an enemy's Defense by %d& for 10 seconds. This skill lasts for 10 seconds.", "Ataca com %d& de chance de reduzir a Defesa do inimigo em %d& por 10 segundos. Dura 10 segundos."},
      {"Attack with a %d& chance of regaining %d& of inflicted damage as HP. This skill lasts for 10 seconds.", "Ataca com %d& de chance de recuperar PV equivalentes a %d& do dano causado. Dura 10 segundos."},
      {"Reduce the MP consumption of all skills by %d& for 10 seconds.", "Reduz o consumo de PM de todas as habilidades em %d& por 10 segundos."},
      {"Increase the Attack Strength of friendly forces by %d& for 10 seconds.", "Aumenta a Forca de Ataque dos aliados em %d& por 10 segundos."},
      {"Increase the Defense of friendly forces by %d& for 10 seconds.", "Aumenta a Defesa dos aliados em %d& por 10 segundos."},
      {"Increase the Evasion Rate of friendly forces by %d& for 10 seconds.", "Aumenta a Taxa de Esquiva dos aliados em %d& por 10 segundos."},
      {"Increase the Critical Attack Chance of friendly forces by %d& for 10 seconds.", "Aumenta a chance de Ataque Critico dos aliados em %d& por 10 segundos."},
      {"Restores my master's HP by %d&.", "Recupera %d& dos PV do meu mestre."},
      {"Increase the Attack Strength of friendly forces by %d&.", "Aumenta a Forca de Ataque dos aliados em %d&."},
      {"Increase your gold drop rate by %d&.", "Aumenta a chance de obter ouro em %d&."},
      {"Increase your item drop rate by %d&.", "Aumenta a chance de obter itens em %d&."},
      {"Increase the Accuracy Rate of friendly forces by %d&.", "Aumenta a Precisao dos aliados em %d&."},
      {"Increase the Evasion Rate of friendly forces by %d&.", "Aumenta a Taxa de Esquiva dos aliados em %d&."},
      {"Increase the Critical Attack Chance of friendly forces by %d&.", "Aumenta a chance de Ataque Critico dos aliados em %d&."},
      {"Increase the HP of friendly forces by %d&.", "Aumenta os PV dos aliados em %d&."},
      {"Increase the MP of friendly forces by %d&.", "Aumenta os PM dos aliados em %d&."},
      {"Noob", "Novato"}, {"Crude", "Rustico"}, {"Simple", "Simples"},
      {"Inferior", "Inferior"}, {"Mediocre", "Medio"}, {"Average", "Medio"},
      {"Superior", "Superior"}, {"Supreme", "Supremo"}, {"Celestial", "Celestial"},
      {"Ultimate", "Supremo"}, {"Normal", "Comum"}, {"Magic", "Magico"},
      {"Rare", "Raro"}, {"Epic", "Epico"}, {"Old", "Antigo"},
      {"Worn", "Gasto"}, {"Iron", "Ferro"}, {"Scale", "Escamas"},
      {"Suede", "Camurca"}, {"Chain", "Malha"}, {"Refined", "Refinado"},
      {"Bone", "Osso"}, {"Silver", "Prata"}, {"Silvery", "Prateado"},
      {"Hunting", "Caca"}, {"Hunter", "Cacador"}, {"Gold", "Ouro"},
      {"Golden", "Dourado"}, {"Fighter", "Guerreiro"}, {"Fight", "Combate"},
      {"Great", "Grande"}, {"Rune", "Runa"}, {"Battle", "Batalha"},
      {"War", "Guerra"}, {"Savage", "Selvagem"}, {"Divine", "Divino"},
      {"Holy", "Sagrado"}, {"Ancient", "Antigo"}, {"Active Volcano", "Vulcao Ativo"},
      {"Dragon Scale", "Escama de Dragao"}, {"Bloodlust", "Sede de Sangue"},
      {"Goddess", "Deusa"}, {"Death", "Morte"}, {"Hell Destroyer", "Destruidor do Inferno"},
      {"Vanquisher", "Vencedor"}, {"Eternal", "Eterno"}, {"Flawless", "Impecavel"},
      {"Sword", "Espada"}, {"Blade", "Lamina"}, {"Gun", "Arma"},
      {"Cannon", "Canhao"}, {"Shooter", "Atirador"}, {"Rifle", "Rifle"},
      {"Armor", "Armadura"}, {"Gloves", "Luvas"}, {"Shoes", "Botas"},
      {"Helm", "Elmo"}, {"Helmet", "Capacete"}, {"Hood", "Capuz"},
      {"Noob Blade", "Lamina de Novato"}, {"Old Blade", "Lamina Antiga"},
      {"Longsword", "Espada Longa"}, {"Saber", "Sabre"},
      {"Zweihander", "Espada de Duas Maos"}, {"Breaker", "Quebrador"},
      {"Moon Blade", "Lamina da Lua"}, {"Venom Slayer", "Matador de Veneno"},
      {"Dark Edge", "Lamina Sombria"}, {"Dark Slayer", "Matador Sombrio"},
      {"Air Blade", "Lamina do Ar"}, {"Ancient Saber", "Sabre Antigo"},
      {"Terror Blade", "Lamina do Terror"}, {"Giant Butcher", "Acougueiro Gigante"},
      {"Punishing Massacrer", "Massacrador Punitivo"}, {"Death Fang", "Presa da Morte"},
      {"Blood Cutter", "Cortador de Sangue"}, {"Destructive Guide", "Guia Destrutivo"},
      {"Meat Tenderizer", "Amaciador de Carne"}, {"Dragon Slayer", "Matador de Dragoes"},
      {"Iron Edge", "Lamina de Ferro"}, {"Broadsword", "Espada Larga"},
      {"Rapier", "Rapieira"}, {"Gladius", "Gladio"}, {"Scimitar", "Cimitarra"},
      {"Steel Blade", "Lamina de Aco"}, {"Glacier sword", "Espada Glacial"},
      {"Volcano sword", "Espada Vulcanica"}, {"Phantom Edge", "Lamina Fantasma"},
      {"Storm sword", "Espada da Tempestade"}, {"Tornado sword", "Espada Tornado"},
      {"Frost Fang", "Presa de Gelo"}, {"Blood Rush", "Investida de Sangue"},
      {"Skull Penetrator", "Perfurador de Cranio"}, {"Soul Destroyer", "Destruidor de Almas"},
      {"Mad Crusader", "Cruzado Louco"}, {"Deathly Guide", "Guia Mortal"},
      {"Raging Blood", "Sangue Furioso"}, {"Raging Storm", "Tempestade Furiosa"},
      {"Bowgun", "Besta"}, {"Musket", "Mosquete"}, {"Sun Rider", "Cavaleiro do Sol"},
      {"Dead Eye", "Olho Morto"}, {"Blaster", "Disparador"},
      {"Drill Shooter", "Atirador Perfurador"}, {"Mega Blaster", "Mega Disparador"},
      {"Giga Shooter", "Atirador Giga"}, {"Accurate Shooter", "Atirador Preciso"},
      {"Lethal Blow", "Golpe Letal"}, {"Demolisher Musket", "Mosquete Demolidor"},
      {"Fiend Crusher", "Esmagador de Demonios"}, {"Hell Grinder", "Moedor Infernal"},
      {"Storm Destroyer", "Destruidor da Tempestade"},
      {"Infinite Crusher", "Esmagador Infinito"}, {"Noob Hat", "Chapeu de Novato"},
      {"Royal Gloves", "Luvas Reais"}, {"Ancient Hands", "Manoplas Antigas"},
      {"Flawless Armor ", "Armadura Impecavel"}, {"Ring of Truth", "Anel da Verdade"},
      {"Ring of Pledge", "Anel do Pacto"}, {"Death Loop", "Elo da Morte"},
      {"Hellish Loop", "Elo Infernal"},
      {"Hat", "Chapeu"}, {"Ring", "Anel"}, {"Hands", "Manoplas"},
      {"Potion", "Pocao"}, {"Vial", "Frasco"}, {"Orb", "Orbe"},
      {"Backpack", "Mochila"}, {"Forging Stone", "Pedra de Forja"},
      {"Appraisal", "Identificacao"}, {"Reset", "Redefinicao"},
      {"Scroll", "Pergaminho"}, {"Special", "Especial"},
      {"Traveler's", "Viajante"}, {"Expert's", "Especialista"},
      {"Appraisal Scroll", "Pergaminho de Identificacao"}, {"Monster Egg", "Ovo de Monstro"},
      {"Return Scroll", "Pergaminho de Retorno"}, {"Orb Removal Scroll", "Pergaminho de Remocao de Orbes"},
      {"Stat Reset Scroll", "Pergaminho de Redefinicao de Atributos"},
      {"Specialty Reset Scroll", "Pergaminho de Redefinicao de Especialidades"},
      {"Jewelry Box", "Caixa de Joias"}, {"Weapon Chest", "Bau de Arma"},
      {"Armor Chest", "Bau de Armadura"}, {"Resurrection Scroll", "Pergaminho de Ressurreicao"},
      {"Recipe", "Receita"}, {"Gem", "Gema"}, {"Mithril", "Mithril"},
      {"Steel", "Aco"}, {"Copper", "Cobre"}, {"Worn Leather", "Couro Gasto"},
      {"Tough Leather", "Couro Resistente"}, {"Durable Leather", "Couro Duravel"},
      {"Magic Leather", "Couro Magico"}, {"Dragon Leather", "Couro de Dragao"},
      {"Linen", "Linho"}, {"Wool", "La"}, {"Silk", "Seda"},
      {"Magic Fabric", "Tecido Magico"}, {"Rune Fabric", "Tecido de Runas"},
      {"Life Plant", "Planta Vital"}, {"Mystic Leaf", "Folha Mistica"},
      {"Magic Plant", "Planta Magica"}, {"Ginseng", "Ginseng"},
      {"Illusion Leaf", "Folha da Ilusao"}, {"Magic Powder", "Po Magico"},
      {"Fairy Powder", "Po de Fada"}, {"Abyss Powder", "Po do Abismo"},
      {"Spirit Powder", "Po Espiritual"}, {"Sky Powder", "Po Celestial"},
      {"Magic Fountain", "Fonte Magica"}, {"Fairy Fountain", "Fonte de Fada"},
      {"Abyss Fountain", "Fonte do Abismo"}, {"Spirit Fountain", "Fonte Espiritual"},
      {"Sky Fountain", "Fonte Celestial"}, {"Magic Essence", "Essencia Magica"},
      {"Fairy Essence", "Essencia de Fada"}, {"Abyss Essence", "Essencia do Abismo"},
      {"Spirit Essence", "Essencia Espiritual"}, {"Sky Essence", "Essencia Celestial"},
      {"Magic Core", "Nucleo Magico"}, {"Fairy Core", "Nucleo de Fada"},
      {"Abyss Core", "Nucleo do Abismo"}, {"Spirit Core", "Nucleo Espiritual"},
      {"Sky Core", "Nucleo Celestial"},
      {"slots.", "espacos."}, {"Instantly", "Instantaneamente"},
      {"equipped", "equipado"}, {"equipping", "equipar"},
      {"create", "criar"}, {"reinforce", "reforcar"},
      {"identify", "identificar"}, {"unidentified", "nao identificado"},
      {"item.", "item."}, {"equipment.", "equipamento."},
      {"Mystic Crystal", "Cristal Mistico"}, {"Shield Breaker Stone", "Pedra Quebra-Escudo"},
      {"Holy Spirit Stone", "Pedra do Espirito Sagrado"}, {"Mushroom Spores", "Esporos de Cogumelo"},
      {"Mushroom Spore", "Esporo de Cogumelo"}, {"Piccolo Skins", "Peles de Piccolo"},
      {"Scorpion Tails", "Caudas de Escorpiao"}, {"Silver Coins", "Moedas de Prata"},
      {"Bone Fragments", "Fragmentos de Osso"}, {"Magic Slimes", "Slimes Magicos"},
      {"Frost Spirits", "Espiritos de Gelo"}, {"Dark Followers", "Seguidores das Trevas"},
      {"Shadow Followers", "Seguidores das Sombras"}, {"Dark Curimons", "Curimons Sombrios"},
      {"Demon Slimes", "Slimes Demonios"}, {"Magic Curimons", "Curimons Magicos"},
      {"Great Worms", "Vermes Gigantes"}, {"King Wasps", "Vespas-Rei"},
      {"Dark Slimes", "Slimes Sombrios"}, {"Deserter Soldiers", "Soldados Desertores"},
      {"Dark Skeletons", "Esqueletos Sombrios"}, {"Magic Skeletons", "Esqueletos Magicos"},
      {"Lycanthropes", "Licantropos"}, {"Demon Lycanthropes", "Licantropos Demonios"},
      {"Mushroom Specimens", "Amostras de Cogumelo"}, {"Insect Specimens", "Amostras de Inseto"},
      {"Venom Stings", "Ferroes Venenosos"}, {"Venom Sting", "Ferrao Venenoso"},
      {"Flame Fragments", "Fragmentos de Chama"}, {"Dark Talismans", "Talismas Sombrios"},
      {"Lost Silver Coins", "Moedas de Prata Perdidas"}, {"Lost Treasure", "Tesouro Perdido"},
      {"Go find ", "Va buscar "}, {"Go back to ", "Volte para "},
      {"Go to the ", "Va para o "}, {"Meet ", "Encontre "},
      {"To the Ancient Ruins", "Rumo as Ruinas Antigas"},
      {"Back to the Camp", "De volta ao Acampamento"},
      {"Deterring the Resurrection", "Impedir a Ressurreicao"},
      {"Hunting Magic Slimes", "Cacando Slimes Magicos"},
      {"Collecting Mushroom Spore", "Coletando Esporos de Cogumelo"},
      {"Taming Pet", "Domando um Mascote"},
      {"Learning Composition", "Aprendendo a Combinar"},
      {"Letter Delivery", "Entrega de Carta"},
      {"Lost Treasure", "Tesouro Perdido"},
      {"Hunting Dark Followers", "Cacando Seguidores das Trevas"},
      {"Exterminating Great Worms", "Eliminando Vermes Gigantes"},
      {"Killing Deserter Soldiers", "Derrotando Soldados Desertores"},
      {"Eliminating Lava Guardian", "Derrotando o Guardiao de Lava"},
      {"Collecting Nippers", "Coletando Alicates"},
      {"Collecting Venom Stings", "Coletando Ferroes Venenosos"},
      {"Collecting Specimens", "Coletando Amostras"},
      {"Subjugating Frost Spirits", "Derrotando Espiritos de Gelo"},
      {"Subjugating Lycanthropes", "Derrotando Licantropos"},
      {"Exterminating King Wasps", "Eliminando Vespas-Rei"},
      {"Exterminating Dark Slimes", "Eliminando Slimes Sombrios"},
      {"Taming", "Domando"}, {"Learning", "Aprendendo"}, {"Composition", "Combinacao"},
      {"Honey", "Mel"}, {"Honeys", "Meles"}, {"Skin", "Pele"}, {"Skins", "Peles"},
      {"Mushroom", "Cogumelo"}, {"Specimens", "Amostras"}, {"Fragment", "Fragmento"},
      {"Fragments", "Fragmentos"}, {"Venom", "Veneno"}, {"Tail", "Cauda"},
      {"Tails", "Caudas"}, {"Coin", "Moeda"}, {"Coins", "Moedas"},
      {"Strength", "Forca"}, {"Dexterity", "Destreza"}, {"Vitality", "Vitalidade"},
      {"Intelligence", "Inteligencia"}, {"Luck", "Sorte"}, {"Appraisal Scroll", "Pergaminho de Identificacao"},
      {"This can be used to identify", "Use para identificar"},
      {"equipment", "equipamento"}, {"equipment item", "equipamento"},
      {"Collect ", "Colete "}, {"Return to ", "Retorne para "}, {"Go to ", "Va para "},
      {"Find ", "Encontre "}, {"Hunt ", "Cace "}, {"Hunt", "Caca"},
      {"Hunting ", "Cacando "}, {"Collecting ", "Coletando "},
      {"Exterminate ", "Elimine "}, {"Subjugate ", "Derrote "},
      {"Deliver ", "Entregue "}, {"Obtain ", "Obtenha "}, {"in the ", " em "},
      {" in ", " em "}, {" near the ", " perto de "}, {" south of ", " ao sul de "},
      {" north of ", " ao norte de "}, {" of the ", " do "},
      {"Friendly forces", "Aliados"}, {"Increase ", "Aumenta "},
      {"Reduce ", "Reduz "}, {"chance of", "chance de"},
      {"Attack and", "Ataque e"}, {"enemies", "inimigos"}, {"enemy", "inimigo"},
      {"surrounding", "ao redor"}, {" for ", " por "}, {" by ", " em "},
      {"%s %d acquired", "%s %d adquirido"},
      {"%s %d& increase", "%s %d& de aumento"},
      {"%s acquired", "%s adquirido"},
      {"%s obtained", "%s obtido"},
      {"%s[LV%d]", "%s[NV%d]"},
      {"(%dW %dL)", "(%dV %dD)"},
      {"(Completed)", "(Concluido)"},
      {"<C%d>%s<C> acquired", "<C%d>%s<C> adquirido"},
      {"<Trial>", "<Teste>"},
      {"??? (Locked)", "??? (Bloqueado)"},
      {"Accuracy", "Precisao"},
      {"Accuracy Orb", "Orbe de Precisao"},
      {"Add", "Adicionar"},
      {"An error has occurred.", "Ocorreu um erro."},
      {"Archer", "Arqueiro"},
      {"Assign Hotkey", "Atribuir Atalho"},
      {"Auction Hall", "Salao de Leiloes"},
      {"Auto Target", "Alvo Automatico"},
      {"Bag", "Mochila"},
      {"Bat", "Morcego"},
      {"Blademaster", "Mestre da Lamina"},
      {"Bruiser", "Brigao"},
      {"Buy :", "Comprar :"},
      {"Buy:", "Comprar:"},
      {"Celestial Accuracy Orb", "Orbe Celestial de Precisao"},
      {"Celestial Critical Orb", "Orbe Celestial de Critico"},
      {"Celestial Evasion Orb", "Orbe Celestial de Evasao"},
      {"Change Class", "Mudar Classe"},
      {"Cheetah", "Guepardo"},
      {"Cobra", "Cobra"},
      {"Completed", "Concluido"},
      {"Compound", "Combinar"},
      {"Cooltime : %dsec", "Recarga : %ds"},
      {"Core", "Nucleo"},
      {"Critical", "Critico"},
      {"Critical Orb", "Orbe de Critico"},
      {"Daily", "Diario"},
      {"Daily Ranking", "Ranking Diario"},
      {"Dark Talisman", "Talisma Sombrio"},
      {"Defeated", "Derrotado"},
      {"Defend", "Defender"},
      {"Details:", "Detalhes:"},
      {"Disassemble", "Desmontar"},
      {"Discard", "Descartar"},
      {"Dolphin", "Golfinho"},
      {"EXP %d acquired", "EXP %d obtido"},
      {"Eagle", "Aguia"},
      {"Elven", "Elfico"},
      {"Enter an amount of gold.", "Insira uma quantidade de ouro."},
      {"Essence", "Essencia"},
      {"Evasion", "Evasao"},
      {"Evasion Orb", "Orbe de Evasao"},
      {"Evolve", "Evoluir"},
      {"Evolve:  <C%d>%s<C>", "Evoluir:  <C%d>%s<C>"},
      {"Fabric", "Tecido"},
      {"Failed", "Falhou"},
      {"Falcon", "Falcao"},
      {"Fast", "Rapido"},
      {"Fountain", "Fonte"},
      {"General", "Geral"},
      {"Gift Item", "Item de Presente"},
      {"Give Up", "Desistir"},
      {"Graphic", "Grafico"},
      {"Grizzly", "Urso Grizzly"},
      {"Gunner", "Atirador"},
      {"Hall of Honor", "Salao da Honra"},
      {"Health", "Saude"},
      {"Herb", "Erva"},
      {"High", "Alto"},
      {"Hippo", "Hipopotamo"},
      {"Honor", "Honra"},
      {"Honor + %d", "Honra + %d"},
      {"Honor Point", "Ponto de Honra"},
      {"Honor Point + %d", "Ponto de Honra + %d"},
      {"Honor Shop", "Loja de Honra"},
      {"Honor:", "Honra:"},
      {"Hotkey", "Atalho"},
      {"Hound", "Cao de Caca"},
      {"Inferior Accuracy Orb", "Orbe de Precisao Inferior"},
      {"Inferior Critical Orb", "Orbe de Critico Inferior"},
      {"Inferior Evasion Orb", "Orbe de Evasao Inferior"},
      {"Infiltrate Lagrant.", "Infiltrar-se em Lagrant."},
      {"Infiltrating the Castle", "Infiltrando-se no Castelo"},
      {"Info", "Informacoes"},
      {"Insect Specimen", "Especime de Inseto"},
      {"Jaguar", "Jaguatirica"},
      {"Killer", "Matador"},
      {"Knight", "Cavaleiro"},
      {"Leather", "Couro"},
      {"Legend", "Lenda"},
      {"Lion", "Leao"},
      {"Lose:", "Derrotas:"},
      {"Low", "Baixo"},
      {"Lynx", "Lince"},
      {"Machine Gun", "Metralhadora"},
      {"Mailbox", "Caixa de Entrada"},
      {"Manage Data", "Gerenciar Dados"},
      {"Master", "Mestre"},
      {"Missing Raina", "Raina Desaparecida"},
      {"Monkey", "Macaco"},
      {"Move", "Mover"},
      {"Multi Mode", "Modo Multi"},
      {"My Info", "Minhas Informacoes"},
      {"My Ranking:", "Meu Ranking:"},
      {"Mystic Crystals", "Cristais Misticos"},
      {"Nipper", "Pinca"},
      {"Nippers", "Pincas"},
      {"Notice", "Aviso"},
      {"Objective:", "Objetivo:"},
      {"Old Gun", "Arma Velha"},
      {"Ore", "Minerio"},
      {"Others", "Outros"},
      {"Panther", "Pantera"},
      {"Peter's Lunchbox", "Lancheira do Peter"},
      {"Please enter a player ID.", "Por favor, insira um ID de jogador."},
      {"Point", "Ponto"},
      {"Powder", "Po"},
      {"Power", "Poder"},
      {"Purchased.", "Comprado."},
      {"Rat", "Rato"},
      {"Received Gift Box", "Caixa de Presente Recebida"},
      {"Recipient : %s", "Destinatario : %s"},
      {"Refuge Camp", "Acampamento de Refugio"},
      {"Refuge Outpost", "Posto de Refugio"},
      {"Register this item?", "Registrar este item?"},
      {"Req. LV:%d", "NV Requisito:%d"},
      {"Require: %s %d Points", "Requer: %s %d Pontos"},
      {"Require: %s 5 Points", "Requer: %s 5 Pontos"},
      {"Required LV:%d", "NV Requerido:%d"},
      {"Resistance Camp", "Acampamento da Resistencia"},
      {"Rewards:", "Recompensas:"},
      {"Rogue", "Picaro"},
      {"Sage's", "do Sabio"},
      {"Saving Raina", "Salvando Raina"},
      {"Saving Urpa", "Salvando Urpa"},
      {"Selling Price : %d", "Preco de Venda : %d"},
      {"Shark", "Tubarao"},
      {"Slime Serum", "Soro de Gosma"},
      {"Slime Serums", "Soros de Gosma"},
      {"Slow", "Lento"},
      {"Sold.", "Vendido."},
      {"Soldier", "Soldado"},
      {"Speed", "Velocidade"},
      {"Success", "Sucesso"},
      {"Superior Accuracy Orb", "Orbe de Precisao Superior"},
      {"Superior Critical Orb", "Orbe de Critico Superior"},
      {"Superior Evasion Orb", "Orbe de Evasao Superior"},
      {"Swiftness", "Agilidade"},
      {"Tech", "Tecnica"},
      {"The character data does not exist.", "Os dados do personagem nao existem."},
      {"The duel will begin shortly.", "O duelo comecara em instantes."},
      {"The game has been saved.", "O jogo foi salvo."},
      {"A network error has occurred.\nError Status = %d", "Ocorreu um erro de rede.\nCodigo de Erro = %d"},
      {"GAMEVIL Inc.\nVer %s\nwww.gamevil.com\n\nIf you have any questions, please use the Customer Inquiry board at our official website.", "GAMEVIL Inc.\nVer %s\nwww.gamevil.com\n\nSe voce tiver alguma duvida, use o painel de Atendimento ao Cliente no nosso site oficial."},
      {"Success. \n<C%d>%s<C> acquired", "Sucesso. \n<C%d>%s<C> adquirido"},
      {"Transferring data...\nPlease do not terminate the game.", "Transferindo dados...\nNao encerre o jogo."},
      {"item has been obtained.\n%s acquired", "item obtido.\n%s adquirido"},
      {"The game has ended.", "O jogo terminou."},
      {"The gift has been sent.", "O presente foi enviado."},
      {"The network is busy. Please try again later.(%d)", "A rede esta ocupada. Tente novamente mais tarde.(%d)"},
      {"The network is not responding.", "A rede nao esta respondendo."},
      {"The recipe is already added.", "A receita ja foi adicionada."},
      {"The requested data does not exist.", "Os dados solicitados nao existem."},
      {"Thief", "Ladrao"},
      {"This bag still contains some items. Please empty it first.", "Esta mochila ainda contem alguns itens. Esvazie-a primeiro."},
      {"This cannot be traded.", "Isto nao pode ser negociado."},
      {"This pet cannot evolve any further.", "Este mascote nao pode evoluir mais."},
      {"This phone number does not exist.", "Este numero de telefone nao existe."},
      {"Thorium", "Torio"},
      {"Thwart the resurrection.", "Impedir a ressurreicao."},
      {"Tiger", "Tigre"},
      {"Titan", "Tita"},
      {"Title acquired:<C9>%s<C>", "Titulo obtido:<C9>%s<C>"},
      {"To Bergheim", "Para Bergheim"},
      {"To Fermion", "Para Fermion"},
      {"To Lagrant", "Para Lagrant"},
      {"To Magus", "Para Magus"},
      {"To Urpa", "Para Urpa"},
      {"Total Ranking", "Ranking Total"},
      {"Transferring data... Please do not terminate the game.", "Transferindo dados... Nao encerre o jogo."},
      {"Transparency", "Transparencia"},
      {"Unidentified ", "Nao Identificado "},
      {"Unidentified item.", "Item nao identificado."},
      {"Urpa's Friend", "Amigo de Urpa"},
      {"Vibration", "Vibracao"},
      {"Weasel", "Doninha"},
      {"Weekly", "Semanal"},
      {"Weekly Ranking", "Ranking Semanal"},
      {"Wickedness", "Maldade"},
      {"Wild", "Selvagem"},
      {"Win:", "Vitorias:"},
      {"Wolf", "Lobo"},
      {"You Won", "Voce Venceu"},
      {"You do not have enough money.", "Voce nao tem dinheiro suficiente."},
      {"You do not meet the stat requirements.", "Voce nao atende aos requisitos de atributos."},
      {"Your bag is full.", "Sua mochila esta cheia."},
      {"Your class has been changed.", "Sua classe foi alterada."},
      {"Your level is too low.", "Seu nivel e muito baixo."},
      {"Your pet's bag is full.", "A mochila do seu mascote esta cheia."},
      {"Your previous bag has more space than the new one.", "Sua mochila anterior tem mais espaco que a nova."},
      {"Your specialties have been reset.", "Suas especialidades foram redefinidas."},
      {"Your stats have been reset.", "Seus atributos foram redefinidos."},
      {"[Data Management]", "[Gerenciamento de Dados]"},
      {"[Manage Data]", "[Gerenciar Dados]"},
      {"[Shop]", "[Loja]"},
  };
  return translations;
}

inline std::string translatePortugueseString(const std::string& source) {
  auto exact = portugueseText().find(source);
  if (exact != portugueseText().end()) return exact->second;
  struct ItemNoun {
    const char* suffix;
    const char* noun;
    bool feminine;
    bool plural;
  };
  static const ItemNoun itemNouns[] = {
      {"Weapon Chest", "Bau de Arma", false, false}, {"Armor Chest", "Bau de Armadura", false, false},
      {"Jewelry Box", "Caixa de Joias", true, false}, {"HP Potion", "Pocao de PV", true, false},
      {"MP Potion", "Pocao de PM", true, false}, {"HP Vial", "Frasco de PV", false, false},
      {"MP Vial", "Frasco de PM", false, false}, {"Gloves", "Luvas", true, true},
      {"Shoes", "Botas", true, true}, {"Armor", "Armadura", true, false},
      {"Sword", "Espada", true, false}, {"Ring", "Anel", false, false},
      {"Helmet", "Capacete", false, false}, {"Helm", "Elmo", false, false},
      {"Hood", "Capuz", false, false}, {"Gun", "Arma", true, false},
      {"Cannon", "Canhao", false, false}, {"Rifle", "Rifle", false, false}};
  for (const auto& item : itemNouns) {
    const std::string suffix = item.suffix;
    if (source.size() <= suffix.size() ||
        source.compare(source.size() - suffix.size(), suffix.size(), suffix) != 0 ||
        source[source.size() - suffix.size() - 1] != ' ')
      continue;

    std::string prefix = source.substr(0, source.size() - suffix.size() - 1);
    static const std::map<std::string, std::string> descriptors = {
        {"Noob", "de Novato"}, {"Crude", "Rustico"}, {"Simple", "Simples"},
        {"Inferior", "Inferior"}, {"Mediocre", "Medio"}, {"Average", "Medio"},
        {"Superior", "Superior"}, {"Supreme", "Supremo"}, {"Celestial", "Celestial"},
        {"Old", "Antigo"}, {"Worn", "Gasto"}, {"Iron", "de Ferro"},
        {"Scale", "de Escamas"}, {"Suede", "de Camurca"}, {"Chain", "de Malha"},
        {"Refined", "Refinado"}, {"Bone", "de Osso"}, {"Silver", "de Prata"},
        {"Silvery", "Prateado"}, {"Hunting", "de Caca"}, {"Hunter", "de Cacador"},
        {"Gold", "de Ouro"}, {"Golden", "Dourado"}, {"Fighter", "de Guerreiro"},
        {"Fight", "de Combate"}, {"Great", "Grande"}, {"Rune", "de Runa"},
        {"Battle", "de Batalha"}, {"War", "de Guerra"}, {"Savage", "Selvagem"},
        {"Divine", "Divino"}, {"Holy", "Sagrado"}, {"Ancient", "Antigo"},
        {"Active Volcano", "do Vulcao Ativo"}, {"Dragon Scale", "de Escama de Dragao"},
        {"Bloodlust", "com Sede de Sangue"}, {"Goddess", "da Deusa"}, {"Zeus", "de Zeus"},
        {"Death", "da Morte"}, {"Hell Destroyer", "do Destruidor do Inferno"},
        {"Vanquisher", "Vencedor"}, {"Eternal", "Eterno"}, {"Flawless", "Impecavel"},
        {"Hand", "de Mao"}, {"Small", "Pequeno"}, {"Noob", "de Novato"},
        {"Infernal Giant", "do Gigante Infernal"}, {"Berserk", "Berserker"},
        {"Knight", "de Cavaleiro"}, {"Dark", "Sombrio"}, {"Flame", "Flamejante"},
        {"Glacier", "Glacial"}, {"Volcano", "Vulcanico"}, {"Storm", "da Tempestade"},
        {"Tornado", "Tornado"}, {"Jagged", "Serrilhado"}, {"Long", "Longa"},
        {"Broad", "Larga"}, {"Bastard", "Bastarda"}, {"Yaksha", "Yaksha"},
        {"Master", "Mestre"}, {"Infernal", "Infernal"}, {"Tazzler", "Atordoante"},
        {"Wyatt", "Wyatt"}, {"Shield", "de Escudo"}, {"Heaven", "Celestial"},
        {"Accurate", "Preciso"}, {"Twister", "Tornado"}, {"Lethal", "Letal"},
        {"Demolisher", "Demolidor"}, {"Anti-frost", "Antigelo"}, {"Fiend", "Demonio"},
        {"Hell", "Infernal"}, {"Infinite", "Infinito"}, {"Spell", "Magico"},
        {"Ice", "de Gelo"}, {"Glass", "de Vidro"}, {"Jade", "de Jade"},
        {"Crystal", "de Cristal"}, {"Cubic", "Cubico"}, {"Sapphire", "de Safira"},
        {"Topaz", "de Topazio"}, {"Magic", "Magico"}, {"Ruby", "de Rubi"},
        {"Diamond", "de Diamante"}, {"Monarch", "do Monarca"}, {"Spiritual", "Espiritual"},
        {"Legend", "Lendario"}, {"Copper", "de Cobre"}, {"Bronze", "de Bronze"}};
    auto descriptor = descriptors.find(prefix);
    if (descriptor == descriptors.end()) continue;
    std::string translatedDescriptor = descriptor->second;
    if (item.feminine) {
      if (prefix == "Crude") translatedDescriptor = "Rustica";
      else if (prefix == "Inferior") translatedDescriptor = "Inferior";
      else if (prefix == "Mediocre" || prefix == "Average") translatedDescriptor = "Media";
      else if (prefix == "Superior") translatedDescriptor = "Superior";
      else if (prefix == "Supreme") translatedDescriptor = "Suprema";
      else if (prefix == "Old") translatedDescriptor = "Antiga";
      else if (prefix == "Worn") translatedDescriptor = "Gasta";
      else if (prefix == "Refined") translatedDescriptor = "Refinada";
      else if (prefix == "Silvery") translatedDescriptor = "Prateada";
      else if (prefix == "Golden") translatedDescriptor = "Dourada";
      else if (prefix == "Savage") translatedDescriptor = "Selvagem";
      else if (prefix == "Divine") translatedDescriptor = "Divina";
      else if (prefix == "Holy") translatedDescriptor = "Sagrada";
      else if (prefix == "Ancient") translatedDescriptor = "Antiga";
      else if (prefix == "Vanquisher") translatedDescriptor = "Vencedora";
      else if (prefix == "Eternal") translatedDescriptor = "Eterna";
      else if (prefix == "Flawless") translatedDescriptor = "Impecavel";
      else if (prefix == "Small") translatedDescriptor = "Pequena";
      else if (prefix == "Bastard") translatedDescriptor = "Bastarda";
      else if (prefix == "Jagged") translatedDescriptor = "Serrilhada";
      else if (prefix == "Long") translatedDescriptor = "Longa";
      else if (prefix == "Broad") translatedDescriptor = "Larga";
      else if (prefix == "Glacier") translatedDescriptor = "Glacial";
      else if (prefix == "Volcano") translatedDescriptor = "Vulcanica";
      else if (prefix == "Accurate") translatedDescriptor = "Precisa";
      else if (prefix == "Lethal") translatedDescriptor = "Letal";
      else if (prefix == "Cubic") translatedDescriptor = "Cubica";
      else if (prefix == "Legend") translatedDescriptor = "Lendaria";
    } else {
      if (prefix == "Crude") translatedDescriptor = "Rustico";
      else if (prefix == "Mediocre" || prefix == "Average") translatedDescriptor = "Medio";
      else if (prefix == "Supreme") translatedDescriptor = "Supremo";
      else if (prefix == "Old") translatedDescriptor = "Antigo";
      else if (prefix == "Worn") translatedDescriptor = "Gasto";
      else if (prefix == "Refined") translatedDescriptor = "Refinado";
      else if (prefix == "Silvery") translatedDescriptor = "Prateado";
      else if (prefix == "Golden") translatedDescriptor = "Dourado";
      else if (prefix == "Divine") translatedDescriptor = "Divino";
      else if (prefix == "Holy") translatedDescriptor = "Sagrado";
      else if (prefix == "Ancient") translatedDescriptor = "Antigo";
      else if (prefix == "Vanquisher") translatedDescriptor = "Vencedor";
      else if (prefix == "Eternal") translatedDescriptor = "Eterno";
      else if (prefix == "Small") translatedDescriptor = "Pequeno";
      else if (prefix == "Flawless") translatedDescriptor = "Impecavel";
      else if (prefix == "Legend") translatedDescriptor = "Lendario";
    }
    if (item.plural) {
      if (prefix == "Crude") translatedDescriptor = "Rusticas";
      else if (prefix == "Inferior") translatedDescriptor = "Inferiores";
      else if (prefix == "Mediocre" || prefix == "Average") translatedDescriptor = "Medias";
      else if (prefix == "Superior") translatedDescriptor = "Superiores";
      else if (prefix == "Supreme") translatedDescriptor = "Supremas";
      else if (prefix == "Celestial") translatedDescriptor = "Celestiais";
      else if (prefix == "Old") translatedDescriptor = "Antigas";
      else if (prefix == "Worn") translatedDescriptor = "Gastas";
      else if (prefix == "Simple") translatedDescriptor = "Simples";
      else if (prefix == "Refined") translatedDescriptor = "Refinadas";
      else if (prefix == "Silvery") translatedDescriptor = "Prateadas";
      else if (prefix == "Golden") translatedDescriptor = "Douradas";
      else if (prefix == "Savage") translatedDescriptor = "Selvagens";
      else if (prefix == "Divine") translatedDescriptor = "Divinas";
      else if (prefix == "Holy") translatedDescriptor = "Sagradas";
      else if (prefix == "Ancient") translatedDescriptor = "Antigas";
      else if (prefix == "Vanquisher") translatedDescriptor = "Vencedoras";
      else if (prefix == "Eternal") translatedDescriptor = "Eternas";
      else if (prefix == "Flawless") translatedDescriptor = "Impecaveis";
      else if (prefix == "Small") translatedDescriptor = "Pequenas";
      else if (prefix == "Great") translatedDescriptor = "Grandes";
    }
    return std::string(item.noun) + " " + translatedDescriptor;
  }

  const std::string produced = "Produces a ";
  if (source.compare(0, produced.size(), produced) == 0) {
    size_t weaponAt = source.find(" weapon (Lv ");
    size_t armorAt = source.find(" armor item (Lv ");
    size_t kindAt = weaponAt != std::string::npos ? weaponAt : armorAt;
    if (kindAt != std::string::npos) {
      std::string grade = source.substr(produced.size(), kindAt - produced.size());
      std::string translatedGrade = translatePortugueseString(grade);
      if (translatedGrade.compare(0, 4, "Raro") == 0 &&
          (translatedGrade.size() == 4 || translatedGrade[4] == ' '))
        translatedGrade.replace(0, 4, "Rara");
      if (!translatedGrade.empty())
        translatedGrade[0] = (char)std::tolower((unsigned char)translatedGrade[0]);
      const size_t typeLength = weaponAt != std::string::npos ? 7 : 11;
      std::string range = source.substr(kindAt + typeLength);
      std::string kind = weaponAt != std::string::npos ? "uma arma " : "uma peca de armadura ";
      return "Produz " + kind + translatedGrade + range;
    }
  }

  static const std::vector<std::pair<std::string, std::string>> ordered = [] {
    std::vector<std::pair<std::string, std::string>> rules(portugueseText().begin(), portugueseText().end());
    std::stable_sort(rules.begin(), rules.end(), [](const auto& left, const auto& right) {
      return left.first.size() > right.first.size();
    });
    return rules;
  }();
  std::string result = source;
  for (const auto& entry : ordered) {
    if ((entry.first.size() < 4 && entry.first != "HP" && entry.first != "MP") ||
        result.find(entry.first) == std::string::npos) continue;
    size_t at = 0;
    while ((at = result.find(entry.first, at)) != std::string::npos) {
      auto isWordCharacter = [](unsigned char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
               (c >= '0' && c <= '9') || c == '_';
      };
      bool leftBoundary = entry.first.empty() || !isWordCharacter((unsigned char)entry.first.front()) ||
                          at == 0 || !isWordCharacter((unsigned char)result[at - 1]);
      size_t end = at + entry.first.size();
      bool rightBoundary = entry.first.empty() || !isWordCharacter((unsigned char)entry.first.back()) ||
                           end == result.size() || !isWordCharacter((unsigned char)result[end]);
      if (!leftBoundary || !rightBoundary) {
        at += entry.first.size();
        continue;
      }
      result.replace(at, entry.first.size(), entry.second);
      at += entry.second.size();
    }
  }
  return result;
}

inline bool translateTable(std::vector<uint8_t>& data) {
  if (data.size() < 2) return false;
  uint16_t count = (uint16_t)data[0] | ((uint16_t)data[1] << 8);
  size_t pos = 2;
  std::vector<uint8_t> translated;
  translated.reserve(data.size() + 8);
  translated.push_back(data[0]);
  translated.push_back(data[1]);
  bool changed = false;

  for (uint16_t i = 0; i < count; ++i) {
    if (pos + 2 > data.size()) return false;
    uint16_t length = (uint16_t)data[pos] | ((uint16_t)data[pos + 1] << 8);
    pos += 2;
    if (pos + length > data.size()) return false;

    const uint8_t* text = data.data() + pos;
    std::string original((const char*)text, length);
    std::string replacement = translatePortugueseString(original);
    if (replacement.size() <= UINT16_MAX && replacement != original) {
      uint16_t newLength = (uint16_t)replacement.size();
      translated.push_back((uint8_t)newLength);
      translated.push_back((uint8_t)(newLength >> 8));
      translated.insert(translated.end(), replacement.begin(), replacement.end());
      changed = true;
    } else {
      translated.push_back((uint8_t)length);
      translated.push_back((uint8_t)(length >> 8));
      translated.insert(translated.end(), text, text + length);
    }
    pos += length;
  }

  if (pos != data.size()) return false;
  if (changed) data = std::move(translated);
  return changed;
}

inline bool translateGdt(const std::string& name, std::vector<uint8_t>& data, const std::function<void(const std::string&)>& note) {
  bool changed = false;
  size_t skippedForCapacity = 0;
  size_t pos = 0;
  while (pos < data.size()) {
    while (pos < data.size() && data[pos] == 0) pos++;
    size_t start = pos;
    while (pos < data.size() && data[pos] != 0) pos++;
    if (start == pos || pos == data.size() || pos - start > 512) continue;
    std::string original((const char*)data.data() + start, pos - start);
    if (std::any_of(original.begin(), original.end(), [](unsigned char c) { return c < 0x20 || c > 0x7e; }))
      continue;
    std::string replacement = translatePortugueseString(original);
    if (replacement == original) continue;
    size_t fieldEnd = pos;
    while (fieldEnd < data.size() && data[fieldEnd] == 0) fieldEnd++;
    if (replacement.size() + 1 > fieldEnd - start) {
      skippedForCapacity++;
      continue;
    }
    std::fill(data.begin() + start, data.begin() + fieldEnd, 0);
    std::copy(replacement.begin(), replacement.end(), data.begin() + start);
    changed = true;
  }
  if (skippedForCapacity)
    note("traducao PT: " + std::to_string(skippedForCapacity) +
         " texto(s) mantido(s) em ingles por falta de espaco em " + name);
  return changed;
}

inline bool isPortugueseAsset(const std::string& name) {
  static const std::vector<std::string> tables = {
      "help_touch_en.tdt", "char_name_en.tdt", "mapname0_en.tdt", "mapname1_en.tdt", "mapname2_en.tdt", "nick_en.tdt",
      "gametxt_en.tdt", "gametxt_f_en.tdt", "equip_name_en.tdt", "equip_head_en.tdt",
      "qitem_name_en.tdt", "skill0_desc_en.tdt", "skill1_desc_en.tdt", "skill2_desc_en.tdt",
      "pet_skill_en.tdt", "attr0_desc_en.tdt", "attr1_desc_en.tdt", "attr2_desc_en.tdt"};
  static const std::vector<std::string> records = {
      "item_en.gdt", "quest_en.gdt", "quest_hard_en.gdt", "mon_data_en.gdt"};
  return std::find(tables.begin(), tables.end(), name) != tables.end() ||
         std::find(records.begin(), records.end(), name) != records.end();
}

inline bool applyPortugueseText(bool enabled, const std::string& name, std::vector<uint8_t>& data, const std::function<void(const std::string&)>& note) {
  if (!enabled || !isPortugueseAsset(name)) return false;
  bool changed = false;
  if (name.size() >= 4 && name.substr(name.size() - 4) == ".tdt") changed = translateTable(data);
  else if (name.size() >= 4 && name.substr(name.size() - 4) == ".gdt") changed = translateGdt(name, data, note);
  if (!changed) note("traducao PT: nenhum texto reconhecido em " + name);
  return changed;
}

} // namespace destinia
