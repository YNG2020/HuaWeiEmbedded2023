function output_txt = customGraphDatatip(~, event_obj, nodeArray, args)
    pos = get(event_obj, 'Position');
    x = pos(1);
    y = pos(2);
    xIDx = floor((x - args.spaceLimX1) / (args.cellX + args.cellXGap)) + 1;
    yIDx = floor((y - args.spaceLimY1) / (args.cellY + args.cellYGap)) + 1;
    nXCell = (args.spaceLimX2 - args.spaceLimX1 - args.cellX) / (args.cellX + args.cellXGap) + 1;
    cellID = (yIDx - 1) * nXCell + xIDx;

    for i = 1 : args.nNodeInCell
        if nodeArray{i, cellID}.nodeCreated
            if nodeArray{i, cellID}.x == x && nodeArray{i, cellID}.y == y
                nodeID = nodeArray{i, cellID}.nodeID;
            end
        end
    end

    % 自定义提示内容
    output_txt = {['nodeID: ',num2str(nodeID)],...
                  ['cellID: ',num2str(cellID)]};
end