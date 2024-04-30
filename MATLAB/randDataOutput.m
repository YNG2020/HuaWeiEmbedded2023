function [] = randDataOutput(args, allEdge, trans)

    outputLocation{1} = "dataMATLAB.txt";
    outputLocation{2} = "..\build\x64-Release\dataMATLAB.txt";
    outputLocation{3} = "..\build\x64-Debug\dataMATLAB.txt";
    outputLocation{4} = "..\build\Linux-GCC-Release\dataMATLAB.txt";
    outputLocation{5} = "..\build\Linux-GCC-Debug\dataMATLAB.txt";
    
    for i = 1 : 5
        % 写入文件
        fileID = fopen(outputLocation{i}, 'w');
        
        % 写入第一行
        fprintf(fileID, '%d %d %d %d %d\n', args.N, args.M, args.T, args.P, args.D);
        
        % 写入边数据
        for j = 1 : args.M
            fprintf(fileID, '%d %d %d\n', allEdge(j, 1), allEdge(j, 2), allEdge(j, 3));
        end
        
        % 写入业务数据
        for j = 1 : args.T
            fprintf(fileID, '%d %d\n', trans(j, 1), trans(j, 2));
        end
        
        % 关闭文件
        fclose(fileID);
    end
end