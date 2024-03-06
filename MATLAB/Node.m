classdef Node
    properties (SetAccess = public)
        x         % x-coordinate
        y         % y-coordinate
        nodeID    % Node ID
        cellID    % cell ID
        nodeCreated % Whether the node is created
    end
    
    methods (Access = public)
        % Constructor
        function obj = Node(x, y, nodeID, cellID)
            if (nargin == 0)
                obj.nodeCreated = false;
            else
                obj.nodeCreated = true;
                obj.x = x;
                obj.y = y;
                obj.nodeID = nodeID;
                obj.cellID = cellID;
            end
        end
    end
end
