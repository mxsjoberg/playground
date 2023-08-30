from mesa import Model, Agent
from mesa.space import SingleGrid
from mesa.time import RandomActivation
from mesa.datacollection import DataCollector
# visualization
from mesa.visualization.ModularVisualization import ModularServer
from mesa.visualization.modules import CanvasGrid, ChartModule, TextElement
from mesa.visualization.UserParam import Slider
# visualization server
from server import server

# schelling agent
class SchellingAgent(Agent):
    def __init__(self, position, model, agent_type):
        super().__init__(position, model)
        self.position = position
        self.type = agent_type
    # step function
    def step(self):
        similar = 0
        # find similar neighbours
        for neighbour in self.model.grid.get_neighbors(self.position, moore=False):
            if neighbour.type == self.type: similar += 1
        # move to random empty location
        if similar < self.model.homophily: self.model.grid.move_to_empty(self)
        else: self.model.happy += 1

# schelling agent model
# https://en.wikipedia.org/wiki/Schelling%27s_model_of_segregation
class Schelling(Model):
    def __init__(self, height, width, density, minority_pc, homophily):
        self.height = height
        self.width = width
        self.density = density
        self.minority_pc = minority_pc
        self.homophily = homophily
        # grid
        self.grid = SingleGrid(height, width, torus=True)
        # schedule
        self.schedule = RandomActivation(self)
        # datacollector
        self.happy = 0
        self.datacollector = DataCollector(
            { "happy": "happy" }, 
            {"x": lambda a: a.position[0], "y": lambda a: a.position[1]}
        )
        # init agents
        for cell in self.grid.coord_iter():
            x = cell[1][0]
            y = cell[1][1]
            if self.random.random() < self.density:
                if self.random.random() < self.minority_pc: agent_type = 1
                else: agent_type = 0
                # create agent
                agent = SchellingAgent((x, y), self, agent_type)
                # add to grid and schedule
                self.grid.place_agent(agent, (x, y))
                self.schedule.add(agent)
        self.running = True
        self.datacollector.collect(self)

    def step(self):
        # reset at each step
        self.happy = 0
        self.schedule.step()
        # collect data
        self.datacollector.collect(self)
        # termination condition : stop if all agents are happy
        if self.happy == self.schedule.get_agent_count(): self.running = False

# visualization
class HappyElement(TextElement):
    def __init__(self): pass
    def render(self, model): return f"happy agents: {model.happy}"

def schelling_draw(agent):
    if agent is None: return
    
    portrayal = { "Shape": "circle", "r": 0.5, "Filled": "true", "Layer": 0 }
    
    if agent.type == 0:
        portrayal["Color"] = ["#FF0000", "#FF9999"]
        portrayal["stroke_color"] = "#00FF00"
    else:
        portrayal["Color"] = ["#0000FF", "#9999FF"]
        portrayal["stroke_color"] = "#000000"

    return portrayal

# create elements
happy_element = HappyElement()
canvas_element = CanvasGrid(schelling_draw, 20, 20, 500, 500)
happy_chart = ChartModule([{ "Label": "happy", "Color": "Black" }])
model_params = {
    "height": 20,
    "width": 20,
    "density": Slider("Agent density", 0.8, 0.1, 1.0, 0.1),
    "minority_pc": Slider("Fraction minority", 0.2, 0.00, 1.0, 0.05),
    "homophily": Slider("Homophily", 3, 0, 8, 1)
}

# launch server
server = ModularServer(Schelling, [canvas_element, happy_element, happy_chart], "Schelling", model_params)
server.launch()
