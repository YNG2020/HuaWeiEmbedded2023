function [x, y] = generateXY(cellId, nodeInCellId, args)
    nXCell = (args.spaceLimX2 - args.spaceLimX1 - args.cellX) / (args.cellX + args.cellXGap) + 1;
    xIdx = mod(cellId - 1, nXCell);
    yIdx = floor((cellId - 1) / nXCell);
    if nodeInCellId == 1 || nodeInCellId == 3
        x = xIdx * (args.cellX + args.cellXGap);
    elseif nodeInCellId == 2 || nodeInCellId == 4
        x = xIdx * (args.cellX + args.cellXGap) + args.cellX;
    end
    if nodeInCellId == 1 || nodeInCellId == 2
        y = yIdx * (args.cellY + args.cellYGap);
    elseif nodeInCellId == 3 || nodeInCellId == 4
        y = yIdx * (args.cellY + args.cellYGap) + args.cellY; 
    end
end