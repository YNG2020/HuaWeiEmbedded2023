function [nodeArray, flatNodeArray] = nodeCreated(args)
    
    rng(args.randomSeed);
    nodeArray = cell(args.nNodeInCell, args.nCell); % 创建一个空的Nodes数组
    nodeID = 0;
    
    for i = 1 : args.nCell
        for j = 1 : args.nNodeInCell
            if rand() >= args.nodeAppearInCell
                tmpNode = Node();
            else
                [x, y] = generateXY(i, j, args);
                tmpNode = Node(x, y, nodeID, i);
                nodeID = nodeID + 1;
            end
            nodeArray{j, i} = tmpNode;
        end
    end
    flatNodeArray = cell(1, nodeID);
    k = 1;
    for i = 1 : args.nCell
        for j = 1 : args.nNodeInCell
            if nodeArray{j, i}.nodeCreated
                flatNodeArray{1, k} = nodeArray{j, i};
                k = k + 1;
            end
        end
    end
    args.N = length(flatNodeArray);
end
